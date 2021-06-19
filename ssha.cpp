#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include "conf.hpp" // provides useraccs_file path

using namespace std;

string create_salt()
{
    constexpr array<char, 10 + 26 * 2> lookup{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                              'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                              'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                              'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                              'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
    random_device rand_dev;
    uniform_int_distribution<int> distr(0, lookup.size() - 1);
    array<char, 16> salt;
    for (auto &i : salt)
        i = lookup[distr(rand_dev)];
    return string(salt.begin(), salt.end());
}

string create_hash(const string &salted_pass)
{
    array<uint8_t, SHA256_DIGEST_LENGTH> pass_enc;
    {
        SHA256_CTX c;
        SHA256_Init(&c);
        SHA256_Update(&c, salted_pass.data(), salted_pass.size());
        SHA256_Final(pass_enc.data(), &c);
    }
    return string(pass_enc.begin(), pass_enc.end());
}

int main(int argc, char **argv)
{
    try
    {
        if (argc != 3)
        {
            cout << "Usage: program_name [username to register] [password]\n";
            return 0;
        }
        const string salt = create_salt();
        ofstream file(useraccs_file, ios::app);
        file.exceptions(ios::failbit | ios::badbit);

        file << argv[1] << " " << create_hash(argv[2] + salt) << " " << salt << "\n";

        cout << "User registration complete.\n";
    }
    catch (const exception &e)
    {
        cerr << e.what() << "\n";
    }
    catch (...)
    {
    }
}
