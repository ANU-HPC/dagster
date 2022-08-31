#include <stdint.h>
#include <stdio.h>

// FROM: Block cipher based security for severely resource-constrained implantable medical devices
//Authors: Christoph Beck, Daniel Masny, Willi Geiselmann, Georg Bretthauer
//
//In section 4.3 in the that paper, there is an IMD protocol that basically runs like this:
//1.       Base -> IMD: ID
//2.       IMD generates random r
//3.       IMD -> Base:  encrypt(r, K_ID)
//4.       Base generates session key Ks
//5.       Base -> IMD: decrypt(r+1, K_ID) || decrypt(Ks XOR r, K_ID)
//6.       IMD extracts the first half of the message in Step 5, encrypts it, and verifies that the “encrypted” matches r+1.   If it does, then the session key is computed by encrypting the second half of the message in Step 5, followed by XOR with r.
//There are follow up steps that confirms the session key Ks, but it’s not important here.
//Here encrypt/decrypt refers to AES-128 encryption in CBC mode. The IMD implements only the decryption function, but given that encryption/decryption are inverses of each other, to save power, the IMD implements only the encryption function, and the base would need to send a “decryption” of the responses (r+1) and (Ks XOR r).  (Note that AES is a bijection, so any random bit string can be decrypted to something).
//The attack I found was a man-in-the-middle attack. The attacker intercepts the message at Step 5, and replaces it with 
//decrypt(r+1, K_ID) || decrypt(r+1, K_ID)
//(i.e., duplicating the first half of the message) .
//This causes the IMD to compute the wrong session key, i.e., it effectively computes Ks = (r+1) XOR r.  The attacker can then guess the correct session key in at most 128 tries (assuming the key size is 128), depending on the value of r.
//For example, if the least significant bit (LSB) of r is 0, then (r+1) + r = 1.
//Generally if the index of the first occurrence of the 0-bit in r is n, then Ks will be 2^n – 1.


#ifdef R128

#define __int256_type unsigned __int128
#define __int128_type uint64_t
#define FULL 64

#elif defined R64

#define __int256_type uint64_t
#define __int128_type uint32_t
#define FULL 32

#elif defined R32

#define __int256_type uint32_t
#define __int128_type uint16_t
#define FULL 16

#else

#define __int256_type uint16_t
#define __int128_type uint8_t
#define FULL 8

#endif




#define COMMA ,
#ifdef UPDEBUG 
	#define DB(x) x
	#define NDB(x) 
#else 
	#define DB(x) 
	#define NDB(x) x
#endif


// custom print functions (if DB is enabled)
DB(template <class T>
void print_binary(T n) {
	while (n) {
		if (n & 1)
			printf("1");
		else
			printf("0");
		n >>= 1;
	}
	printf("\n");
}
template <class T>
void print_annotate(const char* c, T n) {
	printf("%s %li\t",c,n);
	print_binary(n);
})


// undefined function returns for CBMC
__int128_type get_random___int128_type() {}
int get_random_int() {}


// globals
__int128_type ID;
__int128_type KID;
__int128_type r;
__int128_type KS;
__int128_type encrypt_xor;

#define COMMAND_DEPTH 4

uint8_t AI_commands[COMMAND_DEPTH] DB(= {5 COMMA 3 COMMA 2 COMMA 1 COMMA 2 COMMA 0});
__int128_type trials[FULL];



// Helper functions used throughout
__int256_type concatenate(__int128_type x, __int128_type y){
	__int256_type rtn;
	rtn = ( ((__int256_type) x) << FULL) | y;
	return rtn;
}
__int128_type decatenate_2(__int256_type x) { return (__int128_type)x; }
__int128_type decatenate_1(__int256_type x) { return (__int128_type)(x>>FULL); }
__int128_type encrypt(__int128_type key, __int128_type in) { return encrypt_xor ^ in ^ key; } // TODO: full AES here
__int128_type decrypt(__int128_type key, __int128_type in) { return encrypt_xor ^ in ^ key; } // TODO: full AES here



// Base sends IMD its ID
__int128_type Stage1() {
	DB(print_annotate("BASE sends ID:",ID);)
	return ID;
}

// IMD sends encrypted corresponding KID with a randomly generated r
__int128_type Stage2(__int128_type ID) {
	DB(print_annotate("IMD returns encrypted r:",encrypt(KID,r));)
	return encrypt(KID,r);
}
// Base sends 'decrypted' r+1 || 'decrypted' KS ^ r, for random session key KS
__int256_type Stage3(__int128_type EKID_r) {
	__int128_type r = decrypt(KID,EKID_r);
	DB(	print_annotate("BASE inteprets r:",r);
		print_annotate("BASE decrypts r+1:",decrypt(KID,r+1));
		print_annotate("BASE decrypts KS^r:",decrypt(KID,KS^r));)
	__int256_type ret = concatenate(decrypt(KID,r+1),decrypt(KID,KS ^ r));
	DB(	print_annotate("BASE returns concatenation:",ret);)
	return ret;
}

// MITM intercepts stage3, and formats 'decrypted' r+1 || 'decrypted' r+1
__int256_type Stage3_intercept(__int256_type input) {
	// operations: Or, store, shift, and, xor, zero
	__int128_type storage=0;
	for (int i=0; i<COMMAND_DEPTH; i++) {
		//DB(printf("command %i\n",AI_commands[i]);)
		if (AI_commands[i] == 0) { // OR the storage into the input
			//input |= storage;
		} else if (AI_commands[i] == 1) { // store the input into storage
			storage = input;
		} else if (AI_commands[i] == 2) { // rotate input
			input = ((input - ((input >> FULL) << FULL))<<FULL) + (input >> FULL);
		} else if (AI_commands[i] == 3) { // AND the storage into the input
			//input = ((input>>FULL)<<FULL) + storage & input;
		} else if (AI_commands[i] == 4) { // XOR the storage into the input
			//input ^= storage;
		} else if (AI_commands[i] == 5) { // ZERO the storage
			//storage = 0;
		} else if (AI_commands[i] == 6) { // store storage into input
			input = ((input>>FULL)<<FULL) + storage;
		}
		//DB(	printf("input: %i, storage %i\n",input,storage);
		//	print_binary(input);
		//	print_binary(storage);)
	}
	DB(print_annotate("MITM intercepts returning:",input);)
	return input;
}
__int128_type accepted_KS;
// the IMD recieves the MITM signal, and verifies that the r+1 'decryption' was successfully sent,
// and adopts (r+1)^r as the new session key 
int Stage4(__int256_type KS_R) {
	__int128_type a1 = decatenate_1(KS_R);
	DB(print_annotate("IMD recieves part1: ",a1);)
	__int128_type a2 = decatenate_2(KS_R);
	DB(print_annotate("IMD recieves part2: ",a2);)
	accepted_KS = encrypt(KID,a2)^r;
	DB(print_annotate("IMD accepts session key KS:",accepted_KS);)
	__int128_type recieved_r = encrypt(KID,a1);
	DB(print_annotate("IMD decodes r+1 of:",recieved_r);)
	int ret = (recieved_r == (r+1));
	DB(if (ret) {printf("IMD detects good check!\n");} else {printf("IMD detects bad check!\n");})
	return ret;
}
// bassed on the intercepted Stage3 output the MITM, then spams the IMD to get the session key
// MITM knows output from Stage1,2,3 ie. decrypt(KID,r+1) and encrypt(KID,r)
// and also knows that the session key KS= (r+1)^r, for whatever r is.
__int128_type Stage5(int trial) {
	__int128_type ret = (((__int128_type)1)<<(trial+1))-1;
	DB(print_annotate("MITM trying: ",ret);)
	return ret;
}
int Stage6(__int128_type attempt) { // IMD checks if the sessoin key provided by the MITM is valid
	return attempt == accepted_KS;
}


int main() {
	DB(ID = 17844;
	KID = 34343;
	KS = 383834;
	encrypt_xor = 928484;)
	
	NDB(ID = get_random___int128_type();
	KID = get_random___int128_type();
	KS = get_random___int128_type();
	encrypt_xor = get_random___int128_type();)
	
	NDB(for (int i=0; i<COMMAND_DEPTH; i++) {
		AI_commands[i] = get_random_int();
	})
	for (int i=0; i<FULL; i++) {
		DB(trials[i] = Stage5(i);)
		NDB(trials[i] = get_random___int128_type();)
		if (trials[i]==KS) {
			return 0;
		}
	}
	for (int k=0; k<5; k++) {
		DB(print_annotate("setting KS: ",KS);)
		for (int j=0; j< FULL; j+=1) {
			r = (((__int128_type)1)<<j)-1;
			KID += k+j;
			KS += k+j; // introduce some forced randomness into KS&KID
			
			if ((KS^r)==(r+1)) { // exception for degenerate
				DB(printf("exception for degenerate situation\n");)
				return 0;
			}
			
			int session_key_accepted = Stage4(Stage3_intercept(Stage3(Stage2(Stage1()))));
			DB(printf("session key accepted: %i\n", session_key_accepted);)
			if (session_key_accepted) {
				int succeeded = 0;
				for (int i=0; i<FULL; i++) { // assume MITM gets FULL tries at directly guessing KS=(r+1)^r
					if (Stage6(trials[i])) {
						DB(printf("attempt success\n");) // success
						succeeded = 1;
						break;
					} else {
						DB(printf("attempt fail\n");)
					}
				}
				if (succeeded==0) {
					DB(printf("protocol not broken\n");)
					return 0;
				}
			} else {
				DB(printf("protocol not broken, session key not accepted\n");)
				return 0;
			}
		}
	}
	
	
	DB(printf("protocol broken\n");)
	// verification fail if it is possible to reach a failed assert
	// in this context there is a hack that succeeds in violating the protocol
	NDB(__CPROVER_assert(0, "postcondition1");)
}






