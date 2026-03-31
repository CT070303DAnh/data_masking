#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
#using <System.Data.dll>
#using <System.Security.dll>

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Data::Odbc;
using namespace System::Runtime::InteropServices;
using namespace System::Text;
using namespace System::IO;

namespace Core {

// ======================= Cau hinh kich thuoc bo nho =======================
const int MAX_HO_TEN = 256;
const int MAX_SDT = 64;
const int MAX_EMAIL = 320;
const int MAX_CCCD = 64;
const int MAX_DIA_CHI = 600;
const int MAX_KEY = 65;

String^ layChuoiKetNoiDb() {
    return "DSN=CSATBMTT_DSN64;UID=masking_app;PWD=Masking@12345;";
}

struct UserRecord {
    char hoTen[MAX_HO_TEN];
    char soDienThoai[MAX_SDT];
    char email[MAX_EMAIL];
    char cccd[MAX_CCCD];
    char diaChi[MAX_DIA_CHI];
};

int doDaiChuoi(const char chuoi[]) {
    int i = 0;
    while (chuoi[i] != '\0') {
        i = i + 1;
    }
    return i;
}

void saoChepChuoi(char dich[], const char nguon[]) {
    int i = 0;
    while (nguon[i] != '\0') {
        dich[i] = nguon[i];
        i = i + 1;
    }
    dich[i] = '\0';
}

bool laKhoangTrang(char c) {
    return c == ' ';
}

void matNaHoTen(const char hoTenGoc[], char hoTenMasked[]) {
    int i = 0;
    bool dangDauTu = true;
    while (hoTenGoc[i] != '\0') {
        if (laKhoangTrang(hoTenGoc[i])) {
            hoTenMasked[i] = hoTenGoc[i];
            dangDauTu = true;
        }
        else {
            if (dangDauTu) {
                hoTenMasked[i] = hoTenGoc[i];
                dangDauTu = false;
            }
            else {
                hoTenMasked[i] = '*';
            }
        }
        i = i + 1;
    }
    hoTenMasked[i] = '\0';
}

void matNaSoDienThoai(const char sdtGoc[], char sdtMasked[]) {
    int len = doDaiChuoi(sdtGoc);
    int i = 0;
    while (i < len) {
        if (len <= 2) {
            sdtMasked[i] = sdtGoc[i];
        }
        else if (len <= 5) {
            if (i == 0 || i == len - 1) {
                sdtMasked[i] = sdtGoc[i];
            }
            else {
                sdtMasked[i] = '*';
            }
        }
        else {
            if (i < 3 || i >= len - 2) {
                sdtMasked[i] = sdtGoc[i];
            }
            else {
                sdtMasked[i] = '*';
            }
        }
        i = i + 1;
    }
    sdtMasked[len] = '\0';
}

void matNaEmail(const char emailGoc[], char emailMasked[]) {
    int len = doDaiChuoi(emailGoc);
    int viTriA = -1;
    int i = 0;

    while (i < len) {
        if (emailGoc[i] == '@') {
            viTriA = i;
            break;
        }
        i = i + 1;
    }

    if (viTriA == -1) {
        i = 0;
        while (i < len) {
            if (len <= 2) {
                emailMasked[i] = emailGoc[i];
            }
            else if (i == 0 || i == len - 1) {
                emailMasked[i] = emailGoc[i];
            }
            else {
                emailMasked[i] = '*';
            }
            i = i + 1;
        }
        emailMasked[len] = '\0';
        return;
    }

    int soKyTuGiuLocal = 2;
    if (viTriA < 2) {
        soKyTuGiuLocal = viTriA;
    }

    i = 0;
    while (i < len) {
        if (i < viTriA) {
            if (i < soKyTuGiuLocal) {
                emailMasked[i] = emailGoc[i];
            }
            else {
                emailMasked[i] = '*';
            }
        }
        else {
            emailMasked[i] = emailGoc[i];
        }
        i = i + 1;
    }
    emailMasked[len] = '\0';
}

void matNaCCCD(const char cccdGoc[], char cccdMasked[]) {
    int len = doDaiChuoi(cccdGoc);
    int i = 0;
    while (i < len) {
        if (len <= 2) {
            cccdMasked[i] = cccdGoc[i];
        }
        else if (len <= 6) {
            if (i == 0 || i == len - 1) {
                cccdMasked[i] = cccdGoc[i];
            }
            else {
                cccdMasked[i] = '*';
            }
        }
        else {
            if (i < 3 || i >= len - 3) {
                cccdMasked[i] = cccdGoc[i];
            }
            else {
                cccdMasked[i] = '*';
            }
        }
        i = i + 1;
    }
    cccdMasked[len] = '\0';
}

void matNaDiaChi(const char diaChiGoc[], char diaChiMasked[]) {
    int len = doDaiChuoi(diaChiGoc);
    int soKyTuGiu = len / 3;
    if (soKyTuGiu < 6) {
        soKyTuGiu = 6;
    }
    if (soKyTuGiu > len) {
        soKyTuGiu = len;
    }

    int i = 0;
    while (i < len) {
        if (i < soKyTuGiu) {
            diaChiMasked[i] = diaChiGoc[i];
        }
        else {
            if (diaChiGoc[i] == ' ' || diaChiGoc[i] == ',') {
                diaChiMasked[i] = diaChiGoc[i];
            }
            else {
                diaChiMasked[i] = '*';
            }
        }
        i = i + 1;
    }
    diaChiMasked[len] = '\0';
}

void taoBanGhiDaMask(const UserRecord& goc, UserRecord& masked) {
    matNaHoTen(goc.hoTen, masked.hoTen);
    matNaSoDienThoai(goc.soDienThoai, masked.soDienThoai);
    matNaEmail(goc.email, masked.email);
    matNaCCCD(goc.cccd, masked.cccd);
    matNaDiaChi(goc.diaChi, masked.diaChi);
}

char kyTuHex(int n) {
    if (n >= 0 && n <= 9) {
        return (char)('0' + n);
    }
    return (char)('A' + (n - 10));
}

int giaTriHex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

const int DES_IP[64] = {
    58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
};

const int DES_FP[64] = {
    40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
};

const int DES_E[48] = {
    32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,
    12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,
    24,25,26,27,28,29,28,29,30,31,32,1
};

const int DES_P[32] = {
    16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,
    2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25
};

const int DES_PC1[56] = {
    57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35,27,19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,28,20,12,4
};

const int DES_PC2[48] = {
    14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,
    41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32
};

const int DES_SHIFT[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

const int DES_SBOX[8][4][16] = {
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    {
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

unsigned long long permuteBits(unsigned long long input, const int table[], int outLen, int inLen) {
    unsigned long long out = 0;
    int i = 0;
    while (i < outLen) {
        int src = table[i];
        unsigned long long bit = (input >> (inLen - src)) & 1ULL;
        out = (out << 1) | bit;
        i = i + 1;
    }
    return out;
}

unsigned long long bytesToU64(const unsigned char b[8]) {
    unsigned long long x = 0;
    int i = 0;
    while (i < 8) {
        x = (x << 8) | (unsigned long long)b[i];
        i = i + 1;
    }
    return x;
}

void u64ToBytes(unsigned long long x, unsigned char b[8]) {
    int i = 7;
    while (i >= 0) {
        b[i] = (unsigned char)(x & 0xFFULL);
        x = x >> 8;
        i = i - 1;
    }
}

void taoKhoaDES8(const char key[], array<Byte>^ desKey) {
    int i = 0;
    while (i < 8) {
        desKey[i] = (Byte)(0x41 + i);
        i = i + 1;
    }

    int keyLen = doDaiChuoi(key);
    i = 0;
    while (i < keyLen) {
        Byte b = (Byte)((unsigned char)key[i]);
        desKey[i % 8] = (Byte)(desKey[i % 8] ^ b);
        i = i + 1;
    }
}

bool byteArrayToHex(const unsigned char* data, int dataLen, char output[], int outputSize) {
    int j = 0;
    int i = 0;
    while (i < dataLen) {
        if (j + 2 >= outputSize) {
            output[0] = '\0';
            return false;
        }
        unsigned char e = data[i];
        output[j] = kyTuHex((e >> 4) & 0x0F);
        output[j + 1] = kyTuHex(e & 0x0F);
        j = j + 2;
        i = i + 1;
    }
    output[j] = '\0';
    return true;
}

bool hexToByteArray(const char enc[], unsigned char* data, int& outLen, int maxOutLen) {
    int encLen = doDaiChuoi(enc);
    if (encLen % 2 != 0) {
        return false;
    }
    if ((encLen / 2) > maxOutLen) {
        return false;
    }
    outLen = encLen / 2;
    int i = 0;
    int j = 0;
    while (i < encLen) {
        int h1 = giaTriHex(enc[i]);
        int h2 = giaTriHex(enc[i + 1]);
        if (h1 < 0 || h2 < 0) {
            return false;
        }
        data[j] = (unsigned char)((h1 << 4) | h2);
        i = i + 2;
        j = j + 1;
    }
    return true;
}

unsigned int desFeistel(unsigned int r, unsigned long long subKey48) {
    unsigned long long expanded = permuteBits((unsigned long long)r, DES_E, 48, 32);
    unsigned long long x = expanded ^ subKey48;

    unsigned int sboxOut = 0;
    int i = 0;
    while (i < 8) {
        unsigned char block = (unsigned char)((x >> (42 - 6 * i)) & 0x3FULL);
        int row = ((block & 0x20) >> 4) | (block & 0x01);
        int col = (block >> 1) & 0x0F;
        int val = DES_SBOX[i][row][col];
        sboxOut = (sboxOut << 4) | (unsigned int)val;
        i = i + 1;
    }

    return (unsigned int)permuteBits((unsigned long long)sboxOut, DES_P, 32, 32);
}

void desGenerateSubKeys(const unsigned char keyBytes[8], unsigned long long subKeys[16]) {
    unsigned long long key64 = bytesToU64(keyBytes);
    unsigned long long key56 = permuteBits(key64, DES_PC1, 56, 64);
    unsigned int c = (unsigned int)((key56 >> 28) & 0x0FFFFFFF);
    unsigned int d = (unsigned int)(key56 & 0x0FFFFFFF);

    int round = 0;
    while (round < 16) {
        int s = DES_SHIFT[round];
        c = ((c << s) | (c >> (28 - s))) & 0x0FFFFFFF;
        d = ((d << s) | (d >> (28 - s))) & 0x0FFFFFFF;
        unsigned long long cd = ((unsigned long long)c << 28) | (unsigned long long)d;
        subKeys[round] = permuteBits(cd, DES_PC2, 48, 56);
        round = round + 1;
    }
}

void desCryptBlock(const unsigned char in[8], unsigned char out[8], const unsigned long long subKeys[16], bool encrypt) {
    unsigned long long block = bytesToU64(in);
    unsigned long long ip = permuteBits(block, DES_IP, 64, 64);
    unsigned int l = (unsigned int)((ip >> 32) & 0xFFFFFFFFULL);
    unsigned int r = (unsigned int)(ip & 0xFFFFFFFFULL);

    int round = 0;
    while (round < 16) {
        int keyIdx = encrypt ? round : (15 - round);
        unsigned int f = desFeistel(r, subKeys[keyIdx]);
        unsigned int newR = l ^ f;
        l = r;
        r = newR;
        round = round + 1;
    }

    unsigned long long preOut = ((unsigned long long)r << 32) | (unsigned long long)l;
    unsigned long long fp = permuteBits(preOut, DES_FP, 64, 64);
    u64ToBytes(fp, out);
}

bool maHoaDESHex(const char goc[], const char key[], char output[], int outputSize) {
    int keyLen = doDaiChuoi(key);
    if (keyLen <= 0) {
        output[0] = '\0';
        return false;
    }

    int plainLen = doDaiChuoi(goc);
    int padLen = 8 - (plainLen % 8);
    if (padLen == 0) {
        padLen = 8;
    }
    int totalLen = plainLen + padLen;

    unsigned char* plain = new unsigned char[totalLen];
    unsigned char* cipher = new unsigned char[totalLen];
    if (plain == nullptr || cipher == nullptr) {
        if (plain != nullptr) delete[] plain;
        if (cipher != nullptr) delete[] cipher;
        output[0] = '\0';
        return false;
    }

    int i = 0;
    while (i < plainLen) {
        plain[i] = (unsigned char)goc[i];
        i = i + 1;
    }
    while (i < totalLen) {
        plain[i] = (unsigned char)padLen;
        i = i + 1;
    }

    array<Byte>^ keyManaged = gcnew array<Byte>(8);
    taoKhoaDES8(key, keyManaged);
    unsigned char desKey[8];
    i = 0;
    while (i < 8) {
        desKey[i] = (unsigned char)keyManaged[i];
        i = i + 1;
    }

    unsigned long long subKeys[16];
    desGenerateSubKeys(desKey, subKeys);

    unsigned char prev[8];
    i = 0;
    while (i < 8) {
        prev[i] = desKey[i];
        i = i + 1;
    }

    int offset = 0;
    while (offset < totalLen) {
        unsigned char blockIn[8];
        unsigned char blockOut[8];
        i = 0;
        while (i < 8) {
            blockIn[i] = (unsigned char)(plain[offset + i] ^ prev[i]);
            i = i + 1;
        }
        desCryptBlock(blockIn, blockOut, subKeys, true);
        i = 0;
        while (i < 8) {
            cipher[offset + i] = blockOut[i];
            prev[i] = blockOut[i];
            i = i + 1;
        }
        offset = offset + 8;
    }

    bool ok = byteArrayToHex(cipher, totalLen, output, outputSize);
    delete[] plain;
    delete[] cipher;
    if (!ok) {
        output[0] = '\0';
    }
    return ok;
}

bool giaiMaDESHex(const char enc[], const char key[], char output[], int outputSize) {
    int keyLen = doDaiChuoi(key);
    if (keyLen <= 0) {
        return false;
    }

    int maxCipherLen = doDaiChuoi(enc) / 2 + 1;
    unsigned char* cipher = new unsigned char[maxCipherLen];
    if (cipher == nullptr) {
        return false;
    }
    int cipherLen = 0;
    if (!hexToByteArray(enc, cipher, cipherLen, maxCipherLen)) {
        delete[] cipher;
        return false;
    }
    if (cipherLen <= 0 || (cipherLen % 8) != 0) {
        delete[] cipher;
        return false;
    }

    unsigned char* plain = new unsigned char[cipherLen];
    if (plain == nullptr) {
        delete[] cipher;
        return false;
    }

    array<Byte>^ keyManaged = gcnew array<Byte>(8);
    taoKhoaDES8(key, keyManaged);
    unsigned char desKey[8];
    int i = 0;
    while (i < 8) {
        desKey[i] = (unsigned char)keyManaged[i];
        i = i + 1;
    }

    unsigned long long subKeys[16];
    desGenerateSubKeys(desKey, subKeys);

    unsigned char prev[8];
    i = 0;
    while (i < 8) {
        prev[i] = desKey[i];
        i = i + 1;
    }

    int offset = 0;
    while (offset < cipherLen) {
        unsigned char blockIn[8];
        unsigned char blockOut[8];
        i = 0;
        while (i < 8) {
            blockIn[i] = cipher[offset + i];
            i = i + 1;
        }
        desCryptBlock(blockIn, blockOut, subKeys, false);
        i = 0;
        while (i < 8) {
            plain[offset + i] = (unsigned char)(blockOut[i] ^ prev[i]);
            prev[i] = blockIn[i];
            i = i + 1;
        }
        offset = offset + 8;
    }

    int pad = (int)plain[cipherLen - 1];
    if (pad < 1 || pad > 8) {
        delete[] cipher;
        delete[] plain;
        return false;
    }
    i = 0;
    while (i < pad) {
        if (plain[cipherLen - 1 - i] != (unsigned char)pad) {
            delete[] cipher;
            delete[] plain;
            return false;
        }
        i = i + 1;
    }

    int plainLen = cipherLen - pad;
    if (plainLen >= outputSize) {
        delete[] cipher;
        delete[] plain;
        return false;
    }

    i = 0;
    while (i < plainLen) {
        output[i] = (char)plain[i];
        i = i + 1;
    }
    output[plainLen] = '\0';

    delete[] cipher;
    delete[] plain;
    return true;
}

bool maHoaBanGhi(const UserRecord& goc, const char key[], UserRecord& enc) {
    char tempHoTen[600] = { 0 };
    char tempSdt[200] = { 0 };
    char tempEmail[700] = { 0 };
    char tempCccd[200] = { 0 };
    char tempDiaChi[1300] = { 0 };

    if (!maHoaDESHex(goc.hoTen, key, tempHoTen, 600)) {
        return false;
    }
    if (!maHoaDESHex(goc.soDienThoai, key, tempSdt, 200)) {
        return false;
    }
    if (!maHoaDESHex(goc.email, key, tempEmail, 700)) {
        return false;
    }
    if (!maHoaDESHex(goc.cccd, key, tempCccd, 200)) {
        return false;
    }
    if (!maHoaDESHex(goc.diaChi, key, tempDiaChi, 1300)) {
        return false;
    }

    saoChepChuoi(enc.hoTen, tempHoTen);
    saoChepChuoi(enc.soDienThoai, tempSdt);
    saoChepChuoi(enc.email, tempEmail);
    saoChepChuoi(enc.cccd, tempCccd);
    saoChepChuoi(enc.diaChi, tempDiaChi);
    return true;
}

bool giaiMaBanGhi(const UserRecord& enc, const char key[], UserRecord& goc) {
    bool ok = true;
    ok = ok && giaiMaDESHex(enc.hoTen, key, goc.hoTen, MAX_HO_TEN);
    ok = ok && giaiMaDESHex(enc.soDienThoai, key, goc.soDienThoai, MAX_SDT);
    ok = ok && giaiMaDESHex(enc.email, key, goc.email, MAX_EMAIL);
    ok = ok && giaiMaDESHex(enc.cccd, key, goc.cccd, MAX_CCCD);
    ok = ok && giaiMaDESHex(enc.diaChi, key, goc.diaChi, MAX_DIA_CHI);
    return ok;
}

void copyManagedStringToCharBuffer(String^ input, char output[], int outputSize) {
    if (input == nullptr) {
        output[0] = '\0';
        return;
    }
    IntPtr p = Marshal::StringToHGlobalAnsi(input);
    const char* src = (const char*)p.ToPointer();

    int i = 0;
    while (src[i] != '\0' && i < outputSize - 1) {
        output[i] = src[i];
        i = i + 1;
    }
    output[i] = '\0';
    Marshal::FreeHGlobal(p);
}

String^ recordToDisplayString(const UserRecord& u) {
    return "Ho ten: " + gcnew String(u.hoTen) + Environment::NewLine
        + "So dien thoai: " + gcnew String(u.soDienThoai) + Environment::NewLine
        + "Email: " + gcnew String(u.email) + Environment::NewLine
        + "CCCD: " + gcnew String(u.cccd) + Environment::NewLine
        + "Dia chi: " + gcnew String(u.diaChi);
}

unsigned int sha256RotR(unsigned int x, int n) {
    return (x >> n) | (x << (32 - n));
}

unsigned int sha256Ch(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (~x & z);
}

unsigned int sha256Maj(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

unsigned int sha256BSig0(unsigned int x) {
    return sha256RotR(x, 2) ^ sha256RotR(x, 13) ^ sha256RotR(x, 22);
}

unsigned int sha256BSig1(unsigned int x) {
    return sha256RotR(x, 6) ^ sha256RotR(x, 11) ^ sha256RotR(x, 25);
}

unsigned int sha256SSig0(unsigned int x) {
    return sha256RotR(x, 7) ^ sha256RotR(x, 18) ^ (x >> 3);
}

unsigned int sha256SSig1(unsigned int x) {
    return sha256RotR(x, 17) ^ sha256RotR(x, 19) ^ (x >> 10);
}

unsigned int sha256DocBE32(array<Byte>^ data, int offset) {
    return ((unsigned int)data[offset] << 24)
        | ((unsigned int)data[offset + 1] << 16)
        | ((unsigned int)data[offset + 2] << 8)
        | (unsigned int)data[offset + 3];
}

void sha256GhiBE32(unsigned int value, array<Byte>^ data, int offset) {
    data[offset] = (Byte)((value >> 24) & 0xFF);
    data[offset + 1] = (Byte)((value >> 16) & 0xFF);
    data[offset + 2] = (Byte)((value >> 8) & 0xFF);
    data[offset + 3] = (Byte)(value & 0xFF);
}

String^ bamSHA256Hex(String^ noiDung) {
    if (noiDung == nullptr) {
        noiDung = "";
    }

    array<Byte>^ bytes = Encoding::UTF8->GetBytes(noiDung);

    unsigned __int64 bitLen = (unsigned __int64)bytes->Length * 8ULL;
    int paddedLen = bytes->Length + 1 + 8;
    if (paddedLen % 64 != 0) {
        paddedLen = paddedLen + (64 - (paddedLen % 64));
    }

    array<Byte>^ padded = gcnew array<Byte>(paddedLen);
    if (bytes->Length > 0) {
        Buffer::BlockCopy(bytes, 0, padded, 0, bytes->Length);
    }
    padded[bytes->Length] = 0x80;

    int i = 0;
    while (i < 8) {
        padded[paddedLen - 1 - i] = (Byte)((bitLen >> (8 * i)) & 0xFF);
        i = i + 1;
    }

    unsigned int h0 = 0x6a09e667;
    unsigned int h1 = 0xbb67ae85;
    unsigned int h2 = 0x3c6ef372;
    unsigned int h3 = 0xa54ff53a;
    unsigned int h4 = 0x510e527f;
    unsigned int h5 = 0x9b05688c;
    unsigned int h6 = 0x1f83d9ab;
    unsigned int h7 = 0x5be0cd19;

    const unsigned int k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    int offset = 0;
    while (offset < paddedLen) {
        unsigned int w[64];
        int t = 0;
        while (t < 16) {
            w[t] = sha256DocBE32(padded, offset + (t * 4));
            t = t + 1;
        }
        while (t < 64) {
            w[t] = sha256SSig1(w[t - 2]) + w[t - 7] + sha256SSig0(w[t - 15]) + w[t - 16];
            t = t + 1;
        }

        unsigned int a = h0;
        unsigned int b = h1;
        unsigned int c = h2;
        unsigned int d = h3;
        unsigned int e = h4;
        unsigned int f = h5;
        unsigned int g = h6;
        unsigned int h = h7;

        t = 0;
        while (t < 64) {
            unsigned int temp1 = h + sha256BSig1(e) + sha256Ch(e, f, g) + k[t] + w[t];
            unsigned int temp2 = sha256BSig0(a) + sha256Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
            t = t + 1;
        }
        h0 = h0 + a;
        h1 = h1 + b;
        h2 = h2 + c;
        h3 = h3 + d;
        h4 = h4 + e;
        h5 = h5 + f;
        h6 = h6 + g;
        h7 = h7 + h;

        offset = offset + 64;
    }

    array<Byte>^ hash = gcnew array<Byte>(32);
    sha256GhiBE32(h0, hash, 0);
    sha256GhiBE32(h1, hash, 4);
    sha256GhiBE32(h2, hash, 8);
    sha256GhiBE32(h3, hash, 12);
    sha256GhiBE32(h4, hash, 16);
    sha256GhiBE32(h5, hash, 20);
    sha256GhiBE32(h6, hash, 24);
    sha256GhiBE32(h7, hash, 28);

    StringBuilder^ sb = gcnew StringBuilder(hash->Length * 2);
    for each (Byte b in hash) {
        sb->Append(b.ToString("x2"));
    }
    return sb->ToString();
}

void damBaoBangNhatKyTonTaiChung(OdbcConnection^ conn) {
    OdbcCommand^ cmd = gcnew OdbcCommand(
        "CREATE TABLE IF NOT EXISTS NhatKyTruyCap ("
        "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "hanh_dong VARCHAR(50) NOT NULL, "
        "nguoi_dung_id BIGINT UNSIGNED NULL, "
        "ket_qua VARCHAR(20) NOT NULL, "
        "mo_ta VARCHAR(255) NULL, "
        "thoi_gian_tao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ") ENGINE=InnoDB",
        conn
    );
    cmd->ExecuteNonQuery();
}

void ghiNhatKyChung(OdbcConnection^ conn, String^ hanhDong, Nullable<Int64> nguoiDungId, String^ ketQua, String^ moTa) {
    OdbcCommand^ cmd = gcnew OdbcCommand(
        "INSERT INTO NhatKyTruyCap (hanh_dong, nguoi_dung_id, ket_qua, mo_ta) VALUES (?, ?, ?, ?)",
        conn
    );
    cmd->Parameters->AddWithValue("", hanhDong);

    OdbcParameter^ pId = gcnew OdbcParameter();
    pId->OdbcType = OdbcType::BigInt;
    if (nguoiDungId.HasValue) {
        pId->Value = nguoiDungId.Value;
    }
    else {
        pId->Value = DBNull::Value;
    }
    cmd->Parameters->Add(pId);
    cmd->Parameters->AddWithValue("", ketQua);
    cmd->Parameters->AddWithValue("", moTa);
    cmd->ExecuteNonQuery();
}

void damBaoBangTaiKhoanNguoiDungTonTai(OdbcConnection^ conn) {
    OdbcCommand^ cmd = gcnew OdbcCommand(
        "CREATE TABLE IF NOT EXISTS TaiKhoanNguoiDung ("
        "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "ten_dang_nhap VARCHAR(64) NOT NULL UNIQUE, "
        "mat_khau_hash VARCHAR(128) NOT NULL, "
        "vai_tro VARCHAR(20) NOT NULL DEFAULT 'EMPLOYEE', "
        "trang_thai TINYINT NOT NULL DEFAULT 1, "
        "thoi_gian_tao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ") ENGINE=InnoDB",
        conn
    );
    cmd->ExecuteNonQuery();
}

// Forward declarations for functions defined later in this translation unit.
void damBaoBangHoSoNguoiDungTonTai(OdbcConnection^ conn);
void damBaoBangTepTinNguoiDungTonTai(OdbcConnection^ conn);

void damBaoTaiKhoanAdminMacDinhNguoiDung(OdbcConnection^ conn) {
    OdbcCommand^ cmdDem = gcnew OdbcCommand(
        "SELECT COUNT(*) FROM TaiKhoanNguoiDung WHERE ten_dang_nhap = ?",
        conn
    );
    cmdDem->Parameters->AddWithValue("", "admin");
    int soLuong = Convert::ToInt32(cmdDem->ExecuteScalar());
    if (soLuong > 0) {
        return;
    }

    OdbcCommand^ cmdInsert = gcnew OdbcCommand(
        "INSERT INTO TaiKhoanNguoiDung (ten_dang_nhap, mat_khau_hash, vai_tro, trang_thai) VALUES (?, ?, 'ADMIN', 1)",
        conn
    );
    cmdInsert->Parameters->AddWithValue("", "admin");
    cmdInsert->Parameters->AddWithValue("", bamSHA256Hex("Admin@123"));
    cmdInsert->ExecuteNonQuery();
}

bool xacThucTaiKhoanNguoiDung(String^ tenDangNhap, String^ matKhau, String^% thongBao, Int64% userId, String^% vaiTro) {
    thongBao = "Dang nhap that bai.";
    userId = 0;
    vaiTro = nullptr;
    if (String::IsNullOrWhiteSpace(tenDangNhap) || String::IsNullOrWhiteSpace(matKhau)) {
        thongBao = "Vui long nhap day du tai khoan va mat khau.";
        return false;
    }

    String^ tenDangNhapChuan = tenDangNhap->Trim();

    try {
        OdbcConnection^ conn = gcnew OdbcConnection(layChuoiKetNoiDb());
        conn->Open();
        damBaoBangNhatKyTonTaiChung(conn);
        damBaoBangTaiKhoanNguoiDungTonTai(conn);
        damBaoTaiKhoanAdminMacDinhNguoiDung(conn);
        damBaoBangHoSoNguoiDungTonTai(conn);
        damBaoBangTepTinNguoiDungTonTai(conn);

        OdbcCommand^ cmd = gcnew OdbcCommand(
            "SELECT id, mat_khau_hash, vai_tro FROM TaiKhoanNguoiDung WHERE ten_dang_nhap = ? AND trang_thai = 1",
            conn
        );
        cmd->Parameters->AddWithValue("", tenDangNhapChuan);

        OdbcDataReader^ reader = cmd->ExecuteReader();
        if (!reader->Read()) {
            reader->Close();
            ghiNhatKyChung(conn, "LOGIN_FAIL", Nullable<Int64>(), "FAIL", "Tai khoan khong ton tai/bi khoa: " + tenDangNhapChuan);
            conn->Close();
            thongBao = "Tai khoan khong ton tai hoac da bi khoa.";
            return false;
        }

        Int64 idDb = Convert::ToInt64(reader["id"]);
        String^ matKhauHashDb = Convert::ToString(reader["mat_khau_hash"]);
        String^ vaiTroDb = Convert::ToString(reader["vai_tro"]);
        reader->Close();

        String^ matKhauHashNhap = bamSHA256Hex(matKhau);
        bool dungMatKhau = String::Equals(matKhauHashDb, matKhauHashNhap, StringComparison::OrdinalIgnoreCase);

        if (dungMatKhau) {
            userId = idDb;
            vaiTro = vaiTroDb;
            String^ hanhDong = String::Equals(vaiTroDb, "ADMIN", StringComparison::OrdinalIgnoreCase) ? "ADMIN_LOGIN" : "STAFF_LOGIN";
            ghiNhatKyChung(conn, hanhDong, Nullable<Int64>(idDb), "SUCCESS", "Dang nhap thanh cong: " + tenDangNhapChuan);
            conn->Close();
            return true;
        }

        ghiNhatKyChung(conn, "LOGIN_FAIL", Nullable<Int64>(), "FAIL", "Sai mat khau: " + tenDangNhapChuan);
        conn->Close();
        thongBao = "Mat khau khong dung.";
        return false;
    }
    catch (Exception^ ex) {
        thongBao = "Loi xac thuc DB:\n" + ex->Message;
        return false;
    }
}

String^ nhapChuoiDonGian(String^ tieuDe, String^ nhan, bool cheMatKhau) {
    Form^ form = gcnew Form();
    form->Text = tieuDe;
    form->Size = Drawing::Size(420, 170);
    form->StartPosition = FormStartPosition::CenterParent;
    form->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
    form->MaximizeBox = false;
    form->MinimizeBox = false;

    Label^ lb = gcnew Label();
    lb->Text = nhan;
    lb->Location = Point(12, 12);
    lb->Size = Drawing::Size(380, 22);
    form->Controls->Add(lb);

    TextBox^ txt = gcnew TextBox();
    txt->Location = Point(12, 38);
    txt->Size = Drawing::Size(380, 24);
    txt->UseSystemPasswordChar = cheMatKhau;
    form->Controls->Add(txt);

    Button^ ok = gcnew Button();
    ok->Text = "OK";
    ok->Location = Point(226, 74);
    ok->Size = Drawing::Size(80, 30);
    ok->DialogResult = System::Windows::Forms::DialogResult::OK;
    form->Controls->Add(ok);

    Button^ cancel = gcnew Button();
    cancel->Text = "Huy";
    cancel->Location = Point(312, 74);
    cancel->Size = Drawing::Size(80, 30);
    cancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
    form->Controls->Add(cancel);

    form->AcceptButton = ok;
    form->CancelButton = cancel;

    String^ kq = nullptr;
    if (form->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
        kq = txt->Text;
    }
    delete form;
    return kq;
}

void damBaoBangHoSoNguoiDungTonTai(OdbcConnection^ conn) {
    OdbcCommand^ cmd = gcnew OdbcCommand(
        "CREATE TABLE IF NOT EXISTS HoSoNguoiDung ("
        "nguoi_dung_id BIGINT UNSIGNED PRIMARY KEY, "
        "ho_ten_enc VARCHAR(2048) NULL, "
        "so_dien_thoai_enc VARCHAR(1024) NULL, "
        "email_enc VARCHAR(2048) NULL, "
        "cccd_enc VARCHAR(1024) NULL, "
        "dia_chi_enc VARCHAR(4096) NULL, "
        "ho_ten_masked VARCHAR(255) NULL, "
        "so_dien_thoai_masked VARCHAR(64) NULL, "
        "email_masked VARCHAR(320) NULL, "
        "cccd_masked VARCHAR(64) NULL, "
        "dia_chi_masked VARCHAR(600) NULL, "
        "key_hash VARCHAR(128) NULL, "
        "thoi_gian_cap_nhat TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
        "CONSTRAINT fk_hsnd_tknd FOREIGN KEY (nguoi_dung_id) REFERENCES TaiKhoanNguoiDung(id) ON DELETE CASCADE"
        ") ENGINE=InnoDB",
        conn
    );
    cmd->ExecuteNonQuery();
}

void damBaoBangTepTinNguoiDungTonTai(OdbcConnection^ conn) {
    OdbcCommand^ cmd = gcnew OdbcCommand(
        "CREATE TABLE IF NOT EXISTS TepTinNguoiDung ("
        "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
        "ten_file_goc VARCHAR(260) NOT NULL, "
        "duong_dan_luu VARCHAR(1024) NOT NULL, "
        "loai_file VARCHAR(64) NULL, "
        "nguoi_tai_len_id BIGINT UNSIGNED NOT NULL, "
        "da_ma_hoa TINYINT NOT NULL DEFAULT 0, "
        "key_hash VARCHAR(128) NULL, "
        "thoi_gian_tao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "CONSTRAINT fk_teptin_tknd FOREIGN KEY (nguoi_tai_len_id) REFERENCES TaiKhoanNguoiDung(id) ON DELETE CASCADE"
        ") ENGINE=InnoDB",
        conn
    );
    cmd->ExecuteNonQuery();
}

array<Byte>^ maHoaDESDuLieuNhiPhan(array<Byte>^ input, String^ keyText) {
    if (input == nullptr) {
        return gcnew array<Byte>(0);
    }
    if (String::IsNullOrWhiteSpace(keyText)) {
        return nullptr;
    }

    IntPtr p = Marshal::StringToHGlobalAnsi(keyText);
    const char* keyChars = (const char*)p.ToPointer();
    char keyNative[MAX_KEY] = { 0 };
    int i = 0;
    while (keyChars[i] != '\0' && i < MAX_KEY - 1) {
        keyNative[i] = keyChars[i];
        i = i + 1;
    }
    keyNative[i] = '\0';
    Marshal::FreeHGlobal(p);

    unsigned char keyBytes[8];
    array<Byte>^ desKey = gcnew array<Byte>(8);
    taoKhoaDES8(keyNative, desKey);
    i = 0;
    while (i < 8) {
        keyBytes[i] = (unsigned char)desKey[i];
        i = i + 1;
    }

    unsigned long long subKeys[16];
    desGenerateSubKeys(keyBytes, subKeys);

    int plainLen = input->Length;
    int padLen = 8 - (plainLen % 8);
    if (padLen == 0) {
        padLen = 8;
    }
    int totalLen = plainLen + padLen;
    array<Byte>^ output = gcnew array<Byte>(totalLen);
    array<Byte>^ padded = gcnew array<Byte>(totalLen);
    for (i = 0; i < plainLen; i = i + 1) {
        padded[i] = input[i];
    }
    for (i = plainLen; i < totalLen; i = i + 1) {
        padded[i] = (Byte)padLen;
    }

    i = 0;
    while (i < totalLen) {
        unsigned char inBlock[8];
        unsigned char outBlock[8];
        int j = 0;
        while (j < 8) {
            inBlock[j] = (unsigned char)padded[i + j];
            j = j + 1;
        }
        desCryptBlock(inBlock, outBlock, subKeys, true);
        j = 0;
        while (j < 8) {
            output[i + j] = (Byte)outBlock[j];
            j = j + 1;
        }
        i = i + 8;
    }
    return output;
}

array<Byte>^ giaiMaDESDuLieuNhiPhan(array<Byte>^ input, String^ keyText) {
    if (input == nullptr || input->Length == 0 || (input->Length % 8) != 0) {
        return nullptr;
    }
    if (String::IsNullOrWhiteSpace(keyText)) {
        return nullptr;
    }

    IntPtr p = Marshal::StringToHGlobalAnsi(keyText);
    const char* keyChars = (const char*)p.ToPointer();
    char keyNative[MAX_KEY] = { 0 };
    int i = 0;
    while (keyChars[i] != '\0' && i < MAX_KEY - 1) {
        keyNative[i] = keyChars[i];
        i = i + 1;
    }
    keyNative[i] = '\0';
    Marshal::FreeHGlobal(p);

    unsigned char keyBytes[8];
    array<Byte>^ desKey = gcnew array<Byte>(8);
    taoKhoaDES8(keyNative, desKey);
    i = 0;
    while (i < 8) {
        keyBytes[i] = (unsigned char)desKey[i];
        i = i + 1;
    }

    unsigned long long subKeys[16];
    desGenerateSubKeys(keyBytes, subKeys);

    array<Byte>^ dec = gcnew array<Byte>(input->Length);
    i = 0;
    while (i < input->Length) {
        unsigned char inBlock[8];
        unsigned char outBlock[8];
        int j = 0;
        while (j < 8) {
            inBlock[j] = (unsigned char)input[i + j];
            j = j + 1;
        }
        desCryptBlock(inBlock, outBlock, subKeys, false);
        j = 0;
        while (j < 8) {
            dec[i + j] = (Byte)outBlock[j];
            j = j + 1;
        }
        i = i + 8;
    }

    int padLen = (int)dec[dec->Length - 1];
    if (padLen <= 0 || padLen > 8 || padLen > dec->Length) {
        return nullptr;
    }
    i = dec->Length - padLen;
    while (i < dec->Length) {
        if ((int)dec[i] != padLen) {
            return nullptr;
        }
        i = i + 1;
    }

    array<Byte>^ outPlain = gcnew array<Byte>(dec->Length - padLen);
    Buffer::BlockCopy(dec, 0, outPlain, 0, outPlain->Length);
    return outPlain;
}

} // namespace Core
using namespace Core;


#include "component_login_form.h"
#include "component_user_list_form.h"
#include "component_account_management_form.h"
#include "component_profile_form.h"
#include "component_file_management_form.h"
#include "component_audit_log_form.h"
#include "component_main_form.h"

[STAThreadAttribute]
int main(array<String^>^) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    while (true) {
        LoginForm^ manDangNhap = gcnew LoginForm();
        if (manDangNhap->ShowDialog() != System::Windows::Forms::DialogResult::OK) {
            break;
        }

        MainForm^ manChinh = gcnew MainForm(manDangNhap->IdNguoiDungDangNhap, manDangNhap->VaiTroDangNhap);
        Application::Run(manChinh);
        if (!manChinh->YeuCauDangXuat) {
            break;
        }
    }
    return 0;
}
