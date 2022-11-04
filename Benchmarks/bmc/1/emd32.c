/*
 * Code initially written by Luke Croak
 * Modified by Thomas Willingham
 */


#include <stdint.h>

uint64_t encrypt(uint64_t in){
    return in;
}

uint64_t decrypt(uint64_t in){
    return in;
}

// cf. https://stackoverflow.com/questions/12700497/how-to-concatenate-two-integers-in-c
uint64_t concatenate(uint32_t x, uint32_t y){
    uint64_t rtn;
    rtn = ( (uint64_t) x << 32) | y;
    return rtn;
}

struct big_msg{
    uint64_t M1, M2;
};

// cf. https://stackoverflow.com/questions/3200954/what-is-char-bit
struct big_msg interleave(uint64_t x, uint64_t y){
    struct big_msg rtn = {0, 0};

    // Because of size we'll split into two
    uint32_t x_1 = ((((1UL << 32) - 1) << 32) & x) >> 32;
    uint32_t x_2 = ((1UL << 32) - 1) & x;
    uint32_t y_1 = ((((1UL << 32) - 1) << 32) & y) >> 32;
    uint32_t y_2 = ((1UL << 32) - 1) & y;

    for (int i = 0; i < sizeof(x_1) * 8; i++){
        // Pick out appropriate bits
        uint64_t x_1_m = (x_1 & (1UL << i));
        uint64_t y_1_m = (y_1 & (1UL << i));
        // Place them appropriately
        rtn.M1 |= (y_1_m << i);
        rtn.M1 |= (x_1_m << (i+1));
    }

    for (int i = 0; i < sizeof(x_1) * 8; i++){
        // Pick out appropriate bits
        uint64_t x_2_m = (x_2 & (1UL << i));
        uint64_t y_2_m = (y_2 & (1UL << i));
        // Place them appropriately
        rtn.M2 |= (y_2_m << i);
        rtn.M2 |= (x_2_m << (i+1));
    }

    return rtn;
}

struct big_msg inv_interleave(uint64_t x, uint64_t y){
    // First and second un-interleaved components of x and y
    uint32_t x_1 = 0;
    uint32_t x_2 = 0;
    uint32_t y_1 = 0;
    uint32_t y_2 = 0;

    for (int i = 0; i < sizeof(x) * 8 / 2; i ++){ // x first
        uint64_t x_2_m = (x & (1UL << (2*i)));
        uint64_t x_1_m = (x & (1UL << (2*i+1)));
        x_2 |= (x_2_m >> i);
        x_1 |= (x_1_m >> (i+1));
    }

    for (int i = 0; i < sizeof(y) * 8 / 2; i ++){ // y second
        uint64_t y_2_m = (y & (1UL << (2*i)));
        uint64_t y_1_m = (y & (1UL << (2*i+1)));
        y_2 |= (y_2_m >> i);
        y_1 |= (y_1_m >> (i+1));
    }

    uint64_t comb_1 = concatenate(x_1, y_1);
    uint64_t comb_2 = concatenate(x_2, y_2);
    struct big_msg rtn = {comb_1, comb_2};
    return rtn;
}


// Attack is: Base sends legit. Then enemy sends enc_M1 \concat enc_M1.
// This increases the counter on the device to be equal to the serial
// number (e.g. large) presuming the serial number is larger than the
// device counter (which we expect to usually be true). Then base sends
// legit message that is dropped, presuming base counter is smaller than
// serial number (we expect to usually be true), GOAL.

int main(){
    uint64_t X_1; // 64 bit msg
    uint64_t X_2; // 64 bit msg
    uint64_t X_3; // 64 bit msg
    uint32_t B; // 32 bit counters
    uint32_t A;
    uint32_t S_base; // has to be free.
    uint32_t S_device; // has to be free
    _Bool goal = 0;
    uint64_t decrypt_M1_1;
    uint64_t decrypt_M2_1;
    uint64_t decrypt_M1_2;
    uint64_t decrypt_M2_2;
    uint64_t decrypt_M1_3;
    uint64_t decrypt_M2_3;
    struct big_msg tmp_msg_1;
    uint64_t extracted_X_1;
    uint32_t extracted_S_1;
    uint32_t extracted_B_1;
    struct big_msg tmp_msg_2;
    uint64_t extracted_X_2;
    uint32_t extracted_S_2;
    uint32_t extracted_B_2;
    struct big_msg tmp_msg_3;
    uint64_t extracted_X_3;
    uint32_t extracted_S_3;
    uint32_t extracted_B_3;
    _Bool AI_legit_1; // Free, does the attacker make a move.
    _Bool AI_legit_2; // Free, does the attacker make a move.
    _Bool AI_legit_3; // Free, does the attacker make a move.

    // Serial numbers must match
    // TODO: Work out whether should be exit, return, goto the CPROVER assert etc..
    if (S_base != S_device){
        goto exit;  // i.e. clean exit not GOAL
    }

    // Initially counters are equal. Note, I do need the counter to be larger than the serial number for the crash.
    if (A != B){
        goto exit;  // i.e. clean exit not GOAL
    }

    int episodes_counter = 0; // Number of rounds of message sending
    // Buffers with 10 entries, each 64 bits that contains the messages sent for M1 and M2.
    uint64_t episodes_M1[10];
    uint64_t episodes_M2[10];

    if (AI_legit_1){ // For base station
        B += 1; // Increment base station counter

        uint64_t tmp_concat = concatenate(S_base, B);
        struct big_msg msg = interleave(X_1, tmp_concat);
        // e.g. X=[1010]; S=[00]; B=[11]; S \concat B = [0011] ;
        //      [M1;M2] = [1000;1101]

        uint64_t enc_M1 = encrypt(msg.M1);
        uint64_t enc_M2 = encrypt(msg.M2);

        // Store message in buffer for IMD to read from
        episodes_M1[episodes_counter] = enc_M1;
        episodes_M2[episodes_counter] = enc_M2;
        episodes_counter += 1;
    } else { // For attacker
        if (0 == episodes_counter){
            goto exit;  // clean exit cannot make a move until I've seen things over the air.
        }

        // IDEA : Attacker sends message, can only send what it
        //        has seen in the past.

        int AI_index_M1, AI_index_M2; // random - index which episode message to get
        // booleans determine how attacker splices new message together
        _Bool AI_first_M1;
        _Bool AI_first_M2;

        if (0 <= AI_index_M1 && AI_index_M1 < episodes_counter){ // If valid index
            if (AI_first_M1)
                episodes_M1[episodes_counter] = episodes_M1[AI_index_M1]; // Keep M1 referring to an M1
            else
                episodes_M1[episodes_counter] = episodes_M2[AI_index_M1]; // Change M1 to refer to an M2
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
            }

        if (0 <= AI_index_M2 && AI_index_M2 < episodes_counter){ // If valid index
            if (AI_first_M2)
                episodes_M2[episodes_counter] = episodes_M2[AI_index_M2]; // Keep M2 referring to an M2
            else
                episodes_M2[episodes_counter] = episodes_M1[AI_index_M2]; // Change M2 to refer to an M1
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
        }

        episodes_counter += 1;
    }

    decrypt_M1_1 = decrypt(episodes_M1[episodes_counter - 1]); // Read msg from buffer
    decrypt_M2_1 = decrypt(episodes_M2[episodes_counter - 1]);

    tmp_msg_1 = inv_interleave(decrypt_M1_1, decrypt_M2_1);
    extracted_X_1 = tmp_msg_1.M1;
    extracted_S_1 = ((((1 << 32) - 1) << 32) & tmp_msg_1.M2) >> 32;
    extracted_B_1 = ((1 << 32) - 1) & tmp_msg_1.M2;

    if (extracted_S_1 == S_device){
        if (extracted_B_1 < A){
            // GOAL ACHIEVED - since this is never meant to be able to occur and means
            // that the messages sent by the base station will not be accepted.
            goal = 1;
        } else {
            A = extracted_B_1;
        }
    }

    /////////////////////////////////////////////////////////////
    // ROUND 2
    //
    //
    //
    //
    // ////////////////////////////////////////////////////////////

    if (AI_legit_2){ // For base station
        B += 1; // Increment base station counter

        uint64_t tmp_concat = concatenate(S_base, B);
        struct big_msg msg = interleave(X_2, tmp_concat);
        // e.g. X=[1010]; S=[00]; B=[11]; S \concat B = [0011] ;
        //      [M1;M2] = [1000;1101]

        uint64_t enc_M1 = encrypt(msg.M1);
        uint64_t enc_M2 = encrypt(msg.M2);

        // Store message in buffer for IMD to read from
        episodes_M1[episodes_counter] = enc_M1;
        episodes_M2[episodes_counter] = enc_M2;
        episodes_counter += 1;
    } else { // For attacker
        if (0 == episodes_counter){
            goto exit;  // clean exit cannot make a move until I've seen things over the air.
        }

        // IDEA : Attacker sends message, can only send what it
        //        has seen in the past.

        int AI_index_M1, AI_index_M2; // random - index which episode message to get
        // booleans determine how attacker splices new message together
        _Bool AI_first_M1;
        _Bool AI_first_M2;

        if (0 <= AI_index_M1 && AI_index_M1 < episodes_counter){ // If valid index
            if (AI_first_M1)
                episodes_M1[episodes_counter] = episodes_M1[AI_index_M1]; // Keep M1 referring to an M1
            else
                episodes_M1[episodes_counter] = episodes_M2[AI_index_M1]; // Change M1 to refer to an M2
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
            }

        if (0 <= AI_index_M2 && AI_index_M2 < episodes_counter){ // If valid index
            if (AI_first_M2)
                episodes_M2[episodes_counter] = episodes_M2[AI_index_M2]; // Keep M2 referring to an M2
            else
                episodes_M2[episodes_counter] = episodes_M1[AI_index_M2]; // Change M2 to refer to an M1
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
        }

        episodes_counter += 1;
    }

    decrypt_M1_2 = decrypt(episodes_M1[episodes_counter - 1]); // Read msg from buffer
    decrypt_M2_2 = decrypt(episodes_M2[episodes_counter - 1]);

    tmp_msg_2 = inv_interleave(decrypt_M1_2, decrypt_M2_2);
    extracted_X_2 = tmp_msg_2.M1;
    extracted_S_2 = ((((1 << 32) - 1) << 32) & tmp_msg_2.M2) >> 32;
    extracted_B_2 = ((1 << 32) - 1) & tmp_msg_2.M2;

    if (extracted_S_2 == S_device){
        if (extracted_B_2 < A){
            // GOAL ACHIEVED - since this is never meant to be able to occur and means
            // that the messages sent by the base station will not be accepted.
            goal = 1;
        } else {
            A = extracted_B_2;
        }
    }


    /////////////////////////////////////////////////////////////
    // ROUND 3
    //
    //
    //
    //
    // ////////////////////////////////////////////////////////////

    if (AI_legit_3){ // For base station
        B += 1; // Increment base station counter

        uint64_t tmp_concat = concatenate(S_base, B);
        struct big_msg msg = interleave(X_3, tmp_concat);
        // e.g. X=[1010]; S=[00]; B=[11]; S \concat B = [0011] ;
        //      [M1;M2] = [1000;1101]

        uint64_t enc_M1 = encrypt(msg.M1);
        uint64_t enc_M2 = encrypt(msg.M2);

        // Store message in buffer for IMD to read from
        episodes_M1[episodes_counter] = enc_M1;
        episodes_M2[episodes_counter] = enc_M2;
        episodes_counter += 1;
    } else { // For attacker
        if (0 == episodes_counter){
            goto exit;  // clean exit cannot make a move until I've seen things over the air.
        }

        // IDEA : Attacker sends message, can only send what it
        //        has seen in the past.

        int AI_index_M1, AI_index_M2; // random - index which episode message to get
        // booleans determine how attacker splices new message together
        _Bool AI_first_M1;
        _Bool AI_first_M2;

        if (0 <= AI_index_M1 && AI_index_M1 < episodes_counter){ // If valid index
            if (AI_first_M1)
                episodes_M1[episodes_counter] = episodes_M1[AI_index_M1]; // Keep M1 referring to an M1
            else
                episodes_M1[episodes_counter] = episodes_M2[AI_index_M1]; // Change M1 to refer to an M2
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
            }

        if (0 <= AI_index_M2 && AI_index_M2 < episodes_counter){ // If valid index
            if (AI_first_M2)
                episodes_M2[episodes_counter] = episodes_M2[AI_index_M2]; // Keep M2 referring to an M2
            else
                episodes_M2[episodes_counter] = episodes_M1[AI_index_M2]; // Change M2 to refer to an M1
        } else {
            // CLEAN EXIT NO GOAL
            goto exit;
        }

        episodes_counter += 1;
    }

    decrypt_M1_3 = decrypt(episodes_M1[episodes_counter - 1]); // Read msg from buffer
    decrypt_M2_3 = decrypt(episodes_M2[episodes_counter - 1]);

    tmp_msg_3 = inv_interleave(decrypt_M1_3, decrypt_M2_3);
    extracted_X_3 = tmp_msg_3.M1;
    extracted_S_3 = ((((1 << 32) - 1) << 32) & tmp_msg_3.M2) >> 32;
    extracted_B_3 = ((1 << 32) - 1) & tmp_msg_3.M2;

    if (extracted_S_3 == S_device){
        if (extracted_B_3 < A){
            // GOAL ACHIEVED - since this is never meant to be able to occur and means
            // that the messages sent by the base station will not be accepted.
            goal = 1;
        } else {
            A = extracted_B_3;
        }
    }



exit:
    // GO FOR ANOTHER ROUND - SUGGEST UNROLL MANUALLY.
    __CPROVER_assert(goal != 1, "postcondition1");

    // return 0;
}
