// Compile this file with the following command
// g++ Password.cpp -lcrypto -o mdtest

// Example usage:
// ./mdtest "Hello, World!"
// Output Hash:
// dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f


// NOTE: OpenSSL 1.0.2h needs to be installed on the system! It is the LTS solution and will be supported until Dec 2019

// Terminology:
// message = plaintext
// message digest function = hash function
// message digest (MD) = digest = fingerprint = output of a hash function

// The following is based off an example from https://www.openssl.org/docs/man1.0.2/crypto/EVP_DigestInit.html
#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

main(int argc, char *argv[]) {
    EVP_MD_CTX *md_context;
    const EVP_MD *md_function;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;

    OpenSSL_add_all_digests();

    if(!argv[1]) {
        printf("Usage: mdtest message-to-hash digestname\n");
        exit(1);
    }

    if(!argv[2]) {
        printf("Using SHA-256 by default...\n");
        md_function = EVP_sha256();
    }
    else {
        md_function = EVP_get_digestbyname(argv[1]);
        if(!md_function) {
            printf("Unknown message digest function %s\n", argv[2]);
            exit(1);
        }
    }


    printf("Getting hash of string \"%s\"\n", argv[1]);

    md_context = EVP_MD_CTX_create();
    EVP_DigestInit_ex(md_context, md_function, NULL);
    EVP_DigestUpdate(md_context, argv[1], strlen(argv[1]));
    // You can add multiple strings to the message before executing the final digest
    //EVP_DigestUpdate(md_context, mess2, strlen(mess2));
    EVP_DigestFinal_ex(md_context, md_value, &md_len);
    EVP_MD_CTX_destroy(md_context);

    printf("Digest is: \n");
    for(i = 0; i < md_len; i++)
        printf("%02x", md_value[i]);
    printf("\n");

    /* Call this once before exit. */
    EVP_cleanup();
    exit(0);
}


// // #include <openssl/ssl.h>
// #include <openssl/crypto.h>
// // For sha256 hash
// #include <openssl/evp.h>
// // See https://www.openssl.org/docs/manmaster/crypto/EVP_sha512.html


// // TODO: Use uint8_t in place of unsigned char from stdtype?
// // #include <stdtype.h>
// // #include <assert.h>

// /*
// // Pseudocode for salting and stretching a password
// // See pg 304 of Cryptography Engineering (21.2.1 - Salting and Stretching)

// x = 0
// // The salt is simply a random number that is stored alongside the key. Use at least a 256bit salt.
// // Each user needs a different salt, so an attacker would always have to recalculate the key per user,
// // even if the attacker guesses the same password (e.g. "password") for each user.
// salt = rand256()

// for (int i = 0; i < ITERATIONS; ++i) {
//     // (note: || means append)
//     x = hash512(x || password || salt);
// }

// key = x
// // Store the salt and the key in the db. The salt can be public.
// */


// // TODO: Do this once I understand how to use hashing properly
// /* Implementation */

// #define EXTRA_BITS_OF_SECURITY 20;
// // Iterations should be calibrated so the whole process takes 200-1000 ms
// #define ITERATIONS 1 << EXTRA_BITS_OF_SECURITY;

// // TODO: How to hold 256 bits or 512 bits of data? C Array?
// // For 512 bits, thats 512/8 = 64 bytes
// // Create an array to hold 512 bits (64 bytes - 64 chars) of information
// unsigned char x[64];
// // Password can be up to 32 characters (backfill with zeros)
// unsigned char password[32];
// for (int i = 0; i < ITERATIONS; ++i) {
//     const EVP_MD *EVP_sha512(void);

//     x = hash512(x || password || salt);


// }




// // MD = message digest
// EVP_MD_CTX *context = EVP_MD_CTX_new(void);
// // Create a 512-bit SHA hash function on context
// EVP_DigestInit_ex(context, EVP_sha512(), 0);

// /* Or do this:
// EVP_MD_CTX *context;
// EVP_DigestInit(context, EVP_sha512(), 0);
// */


// // TODO: Build up message
// // Don't forget that there is a null character at the end
// char *message = "password";
// size_t count = 8;


// // Hash count bytes of data at d and store in context
// // int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, size_t cnt);
// EVP_DigestUpdate(context, message, count);



// unsigned int *bytes_written;
// unsigned char *message_digest;
// // Retrieve the digest
// // int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md,unsigned int *s);
// EVP_DigestFinal_ex(context, message_digest, bytes_written);


// // Free up the message digest context object
// EVP_MD_CTX_free(context);


// // EVP_DigestFinal() == EVP_DigestFinal_ex() + EVP_MD_CTX_free()

