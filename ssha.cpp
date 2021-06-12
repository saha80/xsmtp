#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <openssl/sha.h>
#include <openssl/rand.h>

using namespace std;

string create_salt()
{
    constexpr char lookup[] = "0123456789"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz";
    constexpr int lookup_size = sizeof(lookup) - 1;
    constexpr int SALT_SIZE = 16;
    array<char, SALT_SIZE> salt;
    srand(time(nullptr));
    for (auto &i : salt)
        i = lookup[rand() % lookup_size];
    return string(salt.begin(), salt.end());
}

string create_hash(const string &salted_pass)
{
    array<uint8_t, SHA256_DIGEST_LENGTH> pass_enc;
    SHA256_CTX c;
    SHA256_Init(&c);
    SHA256_Update(&c, salted_pass.data(), salted_pass.size());
    SHA256_Final(pass_enc.data(), &c);
    return string(pass_enc.begin(), pass_enc.end());
}

int main(int argc, char **argv)
{
    constexpr auto path = "data/users.txt";
    try
    {
        if (argc != 3)
        {
            cout << "Usage: program_name [username to register] [password]\n";
            return 0;
        }
        const string salt = create_salt();
        ofstream file(path, ios::app);
        file.exceptions(ios::failbit | ios::badbit);

        file << argv[1] << ' ' << create_hash(argv[2] + salt) << ' ' << salt << '\n';

        cout << "User registration complete.\n";
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }
}
