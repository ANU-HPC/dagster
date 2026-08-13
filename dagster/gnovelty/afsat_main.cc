/*************************
Copyright 2026

This file is part of Dagster.

Dagster is free software; you can redistribute it
and/or modify it under the terms of the GNU General
Public License as published by the Free Software
Foundation; either version 2 of the License, or
(at your option) any later version.

Dagster is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General
Public License along with Dagster.
If not, see <http://www.gnu.org/licenses/>.
*************************/

#include "afsat_main.hh"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <glog/logging.h>
#include <mpi.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "../CnfHolder.h"
#include "../Dag.h"
#include "../mpi_global.h"
#include "../utilities.h"

using namespace std;

extern CnfHolder *cnf_holder;
extern int world_rank;

// Backlog of buffered stdin bytes beyond which we defer sending a fresh prefix
// to the python helper, rather than growing the buffer unboundedly.
static constexpr size_t PYTHON_WRITE_BACKLOG_LIMIT = 1 * 1024 * 1024; // 1 MB

// If the python helper dies mid-CNF we relaunch it (see design note in the main
// loop). Bound the attempts so a helper that crashes on startup degrades to an
// idle-but-collective-safe state instead of hot-spinning fork/exec.
static constexpr int MAX_PYTHON_RELAUNCHES = 5;

namespace
{

  string trim_copy(const string &s)
  {
    size_t begin = 0;
    while (begin < s.size() && isspace(static_cast<unsigned char>(s[begin])))
      begin++;
    if (begin == s.size())
      return "";
    size_t end = s.size() - 1;
    while (end > begin && isspace(static_cast<unsigned char>(s[end])))
      end--;
    return s.substr(begin, end - begin + 1);
  }

  vector<string> split_csv(const string &s)
  {
    vector<string> parts;
    string token;
    for (char c : s)
    {
      if (c == ',')
      {
        if (!token.empty())
        {
          parts.push_back(token);
          token.clear();
        }
      }
      else if (!isspace(static_cast<unsigned char>(c)))
      {
        token.push_back(c);
      }
    }
    if (!token.empty())
      parts.push_back(token);
    return parts;
  }

  vector<string> detect_nvidia_gpu_indices()
  {
    vector<string> devices;
    FILE *pipe = popen("nvidia-smi --query-gpu=index --format=csv,noheader 2>/dev/null", "r");
    if (!pipe)
      return devices;

    char line[256];
    while (fgets(line, sizeof(line), pipe) != NULL)
    {
      string gpu = trim_copy(line);
      if (!gpu.empty())
        devices.push_back(gpu);
    }
    pclose(pipe);
    return devices;
  }

  string join_csv(const vector<string> &parts)
  {
    string joined;
    for (size_t i = 0; i < parts.size(); i++)
    {
      if (i)
        joined += ",";
      joined += parts[i];
    }
    return joined;
  }

  string shell_quote(const string &raw)
  {
    string out = "'";
    for (char c : raw)
    {
      if (c == '\'')
      {
        out += "'\\''";
      }
      else
      {
        out.push_back(c);
      }
    }
    out += "'";
    return out;
  }

  string activation_command_from_path(const string &python_activate)
  {
    string trimmed = trim_copy(python_activate);
    if (trimmed.empty())
      return "";
    if ((trimmed.rfind("source ", 0) == 0) || (trimmed.rfind(". ", 0) == 0))
      return trimmed;
    return string("source ") + shell_quote(trimmed);
  }

  vector<string> get_compatible_gpu_list(const string &visible_gpus)
  {
    if (visible_gpus != "all")
      return split_csv(visible_gpus);

    const char *inherited = getenv("CUDA_VISIBLE_DEVICES");
    if (inherited && inherited[0] != '\0')
      return split_csv(inherited);

    return detect_nvidia_gpu_indices();
  }

  string select_visible_gpus(
      int helper_rank,
      int helper_count,
      const vector<string> &compatible_gpus,
      int gpus_per_helper)
  {
    if (compatible_gpus.empty())
      return "";

    int per_helper = gpus_per_helper;
    if (per_helper <= 0)
    {
      // Automatic allocation: J//M GPUs per helper.
      per_helper = static_cast<int>(compatible_gpus.size()) / helper_count;
      if (per_helper <= 0)
        per_helper = 1;
    }

    vector<string> assigned;
    assigned.reserve(per_helper);
    // Sequential sliding assignment as requested: helper 0 -> [0,1,...], helper 1 -> [1,2,...].
    for (int i = 0; i < per_helper; i++)
    {
      int idx = (helper_rank + i) % static_cast<int>(compatible_gpus.size());
      assigned.push_back(compatible_gpus[idx]);
    }

    return join_csv(assigned);
  }

  vector<int> parse_literal_payload(istringstream &iss)
  {
    vector<int> ints;
    int v;
    while (iss >> v)
      ints.push_back(v);

    if (ints.empty())
      return ints;

    int count = ints[0];
    if ((count >= 0) && (static_cast<int>(ints.size()) - 1 >= count))
    {
      vector<int> payload;
      payload.reserve(count);
      for (int i = 0; i < count; i++)
        payload.push_back(ints[i + 1]);
      return payload;
    }

    return ints;
  }

  void copy_suggestions(const vector<int> &lits, int *suggestions, int suggestion_size)
  {
    for (int i = 0; i < suggestion_size; i++)
      suggestions[i] = 0;
    int n = min(static_cast<int>(lits.size()), suggestion_size);
    for (int i = 0; i < n; i++)
      suggestions[i] = lits[i];
  }

  void send_solution(MPI_Comm *communicator, const vector<int> &lits, int phase)
  {
    vector<int> packet;
    packet.reserve(lits.size() + 2);
    for (int lit : lits)
      packet.push_back(lit);
    packet.push_back(0);
    packet.push_back(phase);
    MPI_Send(packet.data(), packet.size(), MPI_INT, 0, SLS_SOLUTION_TAG, *communicator);
  }

  void shutdown_prefix_request(MPI_Request *request)
  {
    if ((request == NULL) || (*request == MPI_REQUEST_NULL))
      return;

    int complete = 0;
    MPI_Status status;
    int test_rc = MPI_Test(request, &complete, &status);
    if (test_rc == MPI_SUCCESS && !complete)
    {
      MPI_Cancel(request);
      MPI_Wait(request, &status);
    }

    MPI_Request_free(request);
  }

  class PythonBridge
  {
  public:
    PythonBridge() : pid_(-1), write_fd_(-1), read_fd_(-1) {}

    ~PythonBridge() { stop(); }

    bool launch(const string &command)
    {
      // Fresh process => discard any partial IO left over from a prior instance
      // (relevant when the same object is reused to relaunch a dead helper).
      buffer_.clear();
      write_buffer_.clear();

      int to_child[2];
      int from_child[2];
      if (pipe(to_child) != 0)
        return false;
      if (pipe(from_child) != 0)
      {
        close(to_child[0]);
        close(to_child[1]);
        return false;
      }

      pid_ = fork();
      if (pid_ < 0)
      {
        close(to_child[0]);
        close(to_child[1]);
        close(from_child[0]);
        close(from_child[1]);
        return false;
      }

      if (pid_ == 0)
      {
        dup2(to_child[0], STDIN_FILENO);
        dup2(from_child[1], STDOUT_FILENO);
        dup2(from_child[1], STDERR_FILENO);

        close(to_child[0]);
        close(to_child[1]);
        close(from_child[0]);
        close(from_child[1]);

        execl("/bin/bash", "bash", "-lc", command.c_str(), static_cast<char *>(NULL));
        _exit(127);
      }

      close(to_child[0]);
      close(from_child[1]);
      write_fd_ = to_child[1];
      read_fd_ = from_child[0];

      int flags = fcntl(read_fd_, F_GETFL, 0);
      if (flags >= 0)
        fcntl(read_fd_, F_SETFL, flags | O_NONBLOCK);

      int write_flags = fcntl(write_fd_, F_GETFL, 0);
      if (write_flags >= 0)
        fcntl(write_fd_, F_SETFL, write_flags | O_NONBLOCK);

      return true;
    }

    bool send_line(const string &line)
    {
      write_buffer_ += line;
      write_buffer_ += '\n';
      return flush_writes();
    }

    bool flush_writes()
    {
      if (write_fd_ < 0)
        return write_buffer_.empty();

      while (!write_buffer_.empty())
      {
        ssize_t n = write(write_fd_, write_buffer_.data(), write_buffer_.size());
        if (n > 0)
        {
          write_buffer_.erase(0, n);
        }
        else if (n < 0)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true; // pipe full for now; rest stays buffered, retry next tick
          if (errno == EINTR)
            continue;
          return false; // real error (e.g. EPIPE — python died)
        }
      }
      return true;
    }

    void poll_lines(vector<string> *lines, bool *eof)
    {
      lines->clear();
      *eof = false;
      if (read_fd_ < 0)
        return;

      char buf[4096];
      while (true)
      {
        ssize_t n = read(read_fd_, buf, sizeof(buf));
        if (n > 0)
        {
          buffer_.append(buf, n);
        }
        else if (n == 0)
        {
          *eof = true;
          close(read_fd_);
          read_fd_ = -1;
          break;
        }
        else
        {
          if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            break;
          if (errno == EINTR)
            continue;
          break;
        }
      }

      size_t pos = 0;
      while (true)
      {
        size_t nl = buffer_.find('\n', pos);
        if (nl == string::npos)
          break;
        lines->push_back(buffer_.substr(pos, nl - pos));
        pos = nl + 1;
      }
      if (pos > 0)
        buffer_.erase(0, pos);
    }

    bool is_alive()
    {
      if (pid_ <= 0)
        return false;
      int status = 0;
      pid_t ret = waitpid(pid_, &status, WNOHANG);
      if (ret == 0)
        return true;
      if (ret == pid_)
      {
        pid_ = -1;
        return false;
      }
      return false;
    }

    void stop()
    {
      // Best-effort graceful shutdown: ask python to STOP and give it a moment
      // to exit, then escalate SIGTERM -> SIGKILL. Every wait is bounded so a
      // helper wedged in an uninterruptible GPU call can never hang the rank.
      if (write_fd_ >= 0)
      {
        send_line("STOP");
        flush_writes();
        close(write_fd_);
        write_fd_ = -1;
      }

      if (pid_ > 0)
      {
        if (!reap_bounded(20)) // ~200ms for a clean exit after STOP
        {
          kill(pid_, SIGTERM);
          if (!reap_bounded(20)) // ~200ms after SIGTERM
          {
            kill(pid_, SIGKILL);
            if (!reap_bounded(100)) // ~1s after SIGKILL
            {
              // SIGKILL is uncatchable; if we still cannot reap, the child is
              // stuck in an uninterruptible syscall. Detach rather than block
              // the whole rank — the zombie is reaped on our own exit.
              VLOG(0) << "AFSAT rank " << world_rank
                      << " could not reap python helper pid " << pid_
                      << " after SIGKILL; detaching";
            }
          }
        }
        pid_ = -1;
      }

      if (read_fd_ >= 0)
      {
        close(read_fd_);
        read_fd_ = -1;
      }
    }

    size_t pending_write_bytes() const { return write_buffer_.size(); }

  private:
    // Poll waitpid(WNOHANG) up to `ticks` times (10ms each). Returns true once
    // the child is reaped (or was already gone), false if still running.
    bool reap_bounded(int ticks)
    {
      int status = 0;
      for (int i = 0; i < ticks; i++)
      {
        pid_t ret = waitpid(pid_, &status, WNOHANG);
        if (ret == pid_ || ret < 0)
          return true;
        usleep(10000);
      }
      return false;
    }

    pid_t pid_;
    int write_fd_;
    int read_fd_;
    string buffer_;
    string write_buffer_;
  };

  // Handle one line of python stdout. Returns true if the line carried a
  // SOLUTION (already forwarded to the CDCL process here). Solutions are
  // forwarded and searching continues, mirroring gnovelty's send-and-continue
  // behaviour — a solution is never a reason to leave the helper loop.
  bool process_bridge_line(
      const string &line,
      int *suggestions,
      int suggestion_size,
      MPI_Comm *communicator,
      int phase)
  {
    if (line.empty())
      return false;
    istringstream iss(line);
    string command;
    iss >> command;

    if ((command == "SUGGEST") || (command == "SUGGESTION"))
    {
      vector<int> lits = parse_literal_payload(iss);
      copy_suggestions(lits, suggestions, suggestion_size);
      VLOG(5) << "afsat " << world_rank << " received suggestions from python: count=" << lits.size();
    }
    else if (command == "SOLUTION")
    {
      vector<int> lits = parse_literal_payload(iss);
      if (!lits.empty())
      {
        VLOG(5) << "afsat " << world_rank << " received solution from python: literals=" << lits.size() << " (forwarding to CDCL)";
        send_solution(communicator, lits, phase);
        return true;
      }
    }
    else if (command == "LOG")
    {
      string rest;
      getline(iss, rest);
      VLOG(2) << "AFSAT helper:" << rest;
    }
    else
    {
      VLOG(5) << "afsat " << world_rank << " python output: " << line;
    }
    return false;
  }

  string build_python_command(
      const string &python_executable,
      const string &python_script,
      const string &python_activate,
      const string &cnf_path,
      int helper_rank,
      int helper_count,
      int phase,
      int suggestion_size,
      int gpus_per_helper,
      const string &visible_gpus)
  {
    ostringstream cmd;

    if (!visible_gpus.empty())
    {
      cmd << "export CUDA_VISIBLE_DEVICES=" << shell_quote(visible_gpus) << "; ";
    }
    cmd << "export PYTHONUNBUFFERED=1; ";
    cmd << "export PYTHONIOENCODING='utf-8'; ";

    cmd << "export DAGSTER_CLS_HELPER_RANK=" << helper_rank << "; ";
    cmd << "export DAGSTER_CLS_HELPER_COUNT=" << helper_count << "; ";
    cmd << "export DAGSTER_CLS_WORLD_RANK=" << world_rank << "; ";

    string activation_command = activation_command_from_path(python_activate);
    if (!activation_command.empty())
    {
      cmd << activation_command << " && ";
    }

    cmd << "exec " << shell_quote(python_executable)
        << " -u"
        << " " << shell_quote(python_script)
        << " --cnf " << shell_quote(cnf_path)
        << " --helper-rank " << helper_rank
        << " --helper-count " << helper_count
        << " --world-rank " << world_rank
        << " --phase " << phase
        << " --suggestion-size " << suggestion_size
        << " --debug INFO"
        << " --gpus-per-helper " << gpus_per_helper
        // Hybrid problem files carry native XOR clauses; enable afsat's XOR
        // RREF projection so it exploits them rather than treating them as CNF.
        << " --xor_rref";

    return cmd.str();
  }

} // namespace

int afsat_main(
    MPI_Comm *communicator,
    int suggestionSize,
    const string &python_executable,
    const string &python_script,
    const string &python_activate,
    const string &visible_gpus,
    int gpus_per_helper)
{
  int sub_rank = 0;
  int sub_size = 0;
  MPI_Comm_rank(*communicator, &sub_rank);
  MPI_Comm_size(*communicator, &sub_size);

  // Prevent termination on EPIPE when the helper stdin pipe closes unexpectedly.
  signal(SIGPIPE, SIG_IGN);

  const int helper_rank = sub_rank - 1;
  const int helper_count = sub_size - 1;

  vector<string> compatible_gpus = get_compatible_gpu_list(visible_gpus);
  VLOG(4) << "afsat " << world_rank << " compatible GPU list: "
          << (compatible_gpus.empty() ? string("<none>") : join_csv(compatible_gpus));
  if (compatible_gpus.empty())
  {
    cerr << "AFSAT GPU binding error: no compatible GPUs detected for CLS helpers" << endl;
    MPI_Abort(*communicator, 1);
  }
  if (static_cast<int>(compatible_gpus.size()) < helper_count)
  {
    cerr << "AFSAT GPU binding error: cannot bind " << helper_count
         << " CLS helpers when only " << compatible_gpus.size() << " compatible GPUs are available" << endl;
    MPI_Abort(*communicator, 1);
  }

  while (true)
  {
    // Get a new CNF file from the CDCL process.
    VLOG(3) << "afsat " << world_rank << " waiting for CNF message";
    int fname_length;
    MPI_Status mpi_status;
    MPI_Recv(&fname_length, 1, MPI_INT, 0, CNF_FILENAME_LENGTH_TAG, *communicator, &mpi_status);

    if (fname_length == -1)
      break;
    assert(fname_length > 0);
    VLOG(4) << "afsat " << world_rank << " received startup payload length=" << fname_length;

    int *buffer;
    TEST_NOT_NULL(buffer = (int *)calloc(sizeof(int), fname_length))
    MPI_Recv(buffer, fname_length, MPI_INT, 0, CNF_FILENAME_TAG, *communicator, &mpi_status);
    int phase = buffer[fname_length - 1];
    VLOG(4) << "afsat " << world_rank << " decoded phase=" << phase;

    Message *m = new Message(buffer);
    Cnf *cnf = cnf_holder->compile_Cnf_from_Message(m);
    free(buffer);

    string cnf_path;
    {
      ostringstream path;
      path << "/tmp/dagster_afsat_rank_" << world_rank
           << "_node_" << m->to
           << "_phase_" << phase
           << ".cnf";
      cnf_path = path.str();
    }
    cnf->output_dimacs(cnf_path.c_str());
    VLOG(4) << "afsat " << world_rank << " wrote effective CNF to " << cnf_path;

    // Suggestion window shared with the CDCL process via passive-target RMA.
    // NOTE: MPI_Win_allocate / MPI_Win_free are collective over *communicator
    // and are paired with the master's SatSolver ctor/dtor. The helper must
    // therefore stay in the inner loop below, collective-synchronised, until
    // the master signals a reset with a zero-length prefix. Leaving early (on a
    // solution, or on python death) and reaching MPI_Win_free out of step would
    // free the window under the master's active RMA lock -> hang. This mirrors
    // gnovelty_main, whose inner loop likewise only exits on a zero prefix.
    int *suggestions;
    MPI_Win window;
    VLOG(3) << "afsat " << world_rank << " about to win_allocate";
    auto window_result = MPI_Win_allocate(
        suggestionSize * sizeof(int),
        sizeof(int),
        MPI_INFO_NULL,
        *communicator,
        &suggestions,
        &window);
    if (window_result != MPI_SUCCESS)
      cerr << "UNRECOVERABLE ERROR :: cannot allocate suggestion window from cls helper";

    for (int i = 0; i < suggestionSize; i++)
      suggestions[i] = 0;
    VLOG(4) << "afsat " << world_rank << " allocated and cleared suggestion window";

    string cls_visible_gpus = select_visible_gpus(helper_rank, helper_count, compatible_gpus, gpus_per_helper);
    VLOG(4) << "afsat " << world_rank << " selected CUDA_VISIBLE_DEVICES='"
            << (cls_visible_gpus.empty() ? string("<inherited/all>") : cls_visible_gpus) << "'";
    string python_command = build_python_command(
        python_executable,
        python_script,
        python_activate,
        cnf_path,
        helper_rank,
        helper_count,
        phase,
        suggestionSize,
        gpus_per_helper,
        cls_visible_gpus);
    VLOG(5) << "afsat " << world_rank << " launch command prepared: " << python_command;

    PythonBridge bridge;
    if (!bridge.launch(python_command))
    {
      VLOG(0) << "AFSAT rank " << world_rank << " failed to launch python bridge command";
      cerr << "AFSAT bridge launch failed on rank " << world_rank << endl;
      MPI_Abort(*communicator, 1);
    }
    VLOG(4) << "afsat " << world_rank << " python helper launched";
    bool bridge_alive = true;
    int relaunch_count = 0;

    // The CDCL controller grounds its copy of this CNF (Tseitin encoding of the
    // XOR/PB fragments), so its PREFIX stream lives in the *grounded* variable
    // space: original problem variables plus auxiliary variables. The native PB
    // helper keeps the ungrounded formula, so we must (a) size the receive buffer
    // to the grounded width to avoid MPI_ERR_TRUNCATE, and (b) remember
    // original_vc so auxiliary-variable literals can be dropped before forwarding
    // to python. Cnf::grounded_vc() reports exactly the controller's width without building
    // anything (== original_vc when the node carries no XOR).
    const int original_vc = cnf->vc;
    const int grounded_vc = cnf->grounded_vc();

    // The prefix is a set of assignments the helper cannot change. The persistent
    // receive writes into `prefix`; `pending_prefix` retains the latest one so we
    // can re-send it to a freshly relaunched python helper.
    vector<int> prefix(grounded_vc);
    vector<int> pending_prefix;
    int pending_prefix_length = 0;
    bool has_pending_prefix = false;
    bool python_ready = false;

    MPI_Request prefixRequest;
    MPI_Recv_init(prefix.data(), grounded_vc, MPI_INT, 0, PREFIX_TAG, *communicator, &prefixRequest);
    MPI_Start(&prefixRequest);
    VLOG(4) << "afsat " << world_rank << " prefix listener started";

    // Relaunch python for the current CNF and re-arm the last prefix for send.
    // Returns false once the relaunch budget is exhausted, after which the
    // helper idles (still draining prefixes so the reset can break the loop).
    auto relaunch_python = [&]() -> bool {
      bridge.stop();
      if (relaunch_count >= MAX_PYTHON_RELAUNCHES)
      {
        VLOG(0) << "AFSAT rank " << world_rank
                << " python helper died and relaunch budget exhausted; idling until reset";
        return false;
      }
      relaunch_count++;
      VLOG(1) << "AFSAT rank " << world_rank
              << " python helper died; relaunch attempt " << relaunch_count;
      if (!bridge.launch(python_command))
      {
        VLOG(0) << "AFSAT rank " << world_rank << " python relaunch failed";
        return false;
      }
      python_ready = false;
      if (pending_prefix_length > 0)
        has_pending_prefix = true; // re-send last prefix once python reports READY
      return true;
    };

    // The inner loop. Its ONLY exit is a zero-length prefix from the master,
    // which keeps MPI_Win_free collective-synchronised (see NOTE above).
    while (true)
    {
      if (bridge_alive)
      {
        vector<string> lines;
        bool eof = false;
        // VLOG(5) << "afsat " << world_rank << " polling python helper";
        bridge.poll_lines(&lines, &eof);
        bridge.flush_writes();
        for (const string &line : lines)
        {
          VLOG(5) << "afsat " << world_rank << " polled: " << line;
          if (line.rfind("READY", 0) == 0)
          {
            python_ready = true;
            VLOG(4) << "afsat " << world_rank << " python helper reported READY; prefix forwarding enabled";
            continue;
          }
          process_bridge_line(line, suggestions, suggestionSize, communicator, phase);
        }

        if (eof || !bridge.is_alive())
        {
          VLOG(1) << "AFSAT rank " << world_rank << " python helper stream closed / process exited";
          bridge_alive = relaunch_python();
        }
      }

      // Drain all pending prefix messages, keeping only the most recent.
      bool received_prefix = false;
      int incoming = 0;
      int prefix_length = 0;
      MPI_Status prefixStatus;
      MPI_Test(&prefixRequest, &incoming, &prefixStatus);
      while (incoming)
      {
        received_prefix = true;
        MPI_Get_count(&prefixStatus, MPI_INT, &prefix_length);
        if (prefix_length < 0 || prefix_length > grounded_vc)
        {
          cerr << "AFSAT prefix error: received prefix length " << prefix_length
               << " exceeds grounded variable count " << grounded_vc << endl;
          MPI_Abort(*communicator, 1);
        }
        if (prefix_length == 0)
          break;
        MPI_Start(&prefixRequest);
        MPI_Test(&prefixRequest, &incoming, &prefixStatus);
      }

      if (received_prefix)
      {
        VLOG(5) << "afsat " << world_rank << " received prefix length=" << prefix_length;
        if (prefix_length == 0)
        {
          // Zero-length prefix: master is resetting this helper for the next
          // CNF. This is the sole, collective-safe exit from the inner loop.
          VLOG(3) << "afsat " << world_rank << " received reset prefix; leaving helper loop";
          break;
        }

        // Forward only original-variable literals. Auxiliary variables
        // (|var| > original_vc) are the controller's internal Tseitin vars: the
        // native PB helper re-derives XOR consistency via RREF and would overflow
        // python's assignment vector (sized to original_vc) if handed them. An
        // all-auxiliary prefix filters to empty and is simply not forwarded (a
        // zero-length PREFIX would be misread by python as STOP).
        pending_prefix.clear();
        for (int i = 0; i < prefix_length; i++)
          if (abs(prefix[i]) <= original_vc)
            pending_prefix.push_back(prefix[i]);
        pending_prefix_length = (int)pending_prefix.size();
        has_pending_prefix = (pending_prefix_length > 0);
        if (has_pending_prefix && !python_ready)
          VLOG(5) << "afsat " << world_rank << " queued latest prefix until python READY: length=" << pending_prefix_length;
      }

      if (bridge_alive && python_ready && has_pending_prefix)
      {
        if (bridge.pending_write_bytes() > PYTHON_WRITE_BACKLOG_LIMIT)
        {
          // VLOG(1) << "AFSAT rank " << world_rank << " python write backlog "
          //         << bridge.pending_write_bytes() << "; deferring prefix send";
        }
        else
        {
          ostringstream prefix_line;
          prefix_line << "PREFIX " << pending_prefix_length;
          for (int i = 0; i < pending_prefix_length; i++)
            prefix_line << " " << pending_prefix[i];
          VLOG(5) << "afsat " << world_rank << " sending prefix to python: length=" << pending_prefix_length;
          if (bridge.send_line(prefix_line.str()))
          {
            has_pending_prefix = false;
          }
          else
          {
            // Write failed => python died; relaunch will re-arm this prefix.
            VLOG(1) << "AFSAT rank " << world_rank << " failed to send prefix; python helper appears dead";
            bridge_alive = relaunch_python();
          }
        }
      }

      usleep(1000);
    }

    shutdown_prefix_request(&prefixRequest);
    bridge.stop();
    VLOG(2) << "afsat " << world_rank << " python helper stopped and prefix listener closed";

    MPI_Win_free(&window);
    unlink(cnf_path.c_str());
    VLOG(2) << "afsat " << world_rank << " cleaned up suggestion window and temporary CNF file";

    delete cnf;
    delete m;
  }

  VLOG(2) << "CLS helper loop terminated";
  return 0;
}
