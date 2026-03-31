#include <iostream>
#include <limits>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Fallback type cho mot so moi truong build cl.exe trong VS Code
// (tranh loi parse sqltypes/sqlext khi thieu khai bao he thong).
#ifndef INT64
typedef __int64 INT64;
#endif
#ifndef UINT64
typedef unsigned __int64 UINT64;
#endif
#ifndef LPWSTR
typedef wchar_t* LPWSTR;
#endif

#include <sql.h>
#include <sqlext.h>

// Link thu vien ODBC khi build tren Visual Studio
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

using namespace std;

// ======================= Cau hinh kich thuoc bo nho =======================
const int MAX_HO_TEN = 101;
const int MAX_SDT = 21;
const int MAX_EMAIL = 121;
const int MAX_CCCD = 21;
const int MAX_DIA_CHI = 256;

// ======================= Cau truc du lieu nguoi dung =======================
struct UserRecord {
    char hoTen[MAX_HO_TEN];
    char soDienThoai[MAX_SDT];
    char email[MAX_EMAIL];
    char cccd[MAX_CCCD];
    char diaChi[MAX_DIA_CHI];
};

// ======================= Ham xu ly chuoi tu cai dat =======================
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
    if (c == ' ') {
        return true;
    }
    return false;
}

// ======================= Cac ham Data Masking =======================
void matNaHoTen(const char hoTenGoc[], char hoTenMasked[]) {
    int i = 0;
    bool dangDauTu = true;

    while (hoTenGoc[i] != '\0') {
        if (laKhoangTrang(hoTenGoc[i])) {
            hoTenMasked[i] = hoTenGoc[i];
            dangDauTu = true;
        }
        else {
            if (dangDauTu == true) {
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

    // Tim ky tu '@' thu cong
    while (i < len) {
        if (emailGoc[i] == '@') {
            viTriA = i;
            break;
        }
        i = i + 1;
    }

    // Neu email khong hop le thi mask chung: giu dau + cuoi
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

    // Co '@': giu 2 ky tu dau local-part, mask phan local con lai
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

// ======================= Ham hien thi loi ODBC =======================
void inChanDoanODBC(SQLSMALLINT handleType, SQLHANDLE handle, const char* moTa) {
    cout << "[ODBC ERROR] " << moTa << "\n";

    SQLCHAR sqlState[6];
    SQLCHAR message[256];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;
    SQLSMALLINT i = 1;

    while (SQLGetDiagRecA(
        handleType,
        handle,
        i,
        sqlState,
        &nativeError,
        message,
        sizeof(message),
        &textLength) == SQL_SUCCESS)
    {
        cout << "  SQLSTATE: " << sqlState << " | NativeError: " << nativeError
            << " | Message: " << message << "\n";
        i = i + 1;
    }
}

bool thanhCong(SQLRETURN rc) {
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
        return true;
    }
    return false;
}

// ======================= Nhap lieu nguoi dung =======================
void nhapThongTinNguoiDung(UserRecord& u) {
    cout << "Nhap ho ten: ";
    cin.getline(u.hoTen, MAX_HO_TEN);

    cout << "Nhap so dien thoai: ";
    cin.getline(u.soDienThoai, MAX_SDT);

    cout << "Nhap email: ";
    cin.getline(u.email, MAX_EMAIL);

    cout << "Nhap CCCD: ";
    cin.getline(u.cccd, MAX_CCCD);

    cout << "Nhap dia chi: ";
    cin.getline(u.diaChi, MAX_DIA_CHI);
}

void inBanGhi(const char* tieuDe, const UserRecord& u) {
    cout << "\n=== " << tieuDe << " ===\n";
    cout << "Ho ten      : " << u.hoTen << "\n";
    cout << "So dien thoai: " << u.soDienThoai << "\n";
    cout << "Email       : " << u.email << "\n";
    cout << "CCCD        : " << u.cccd << "\n";
    cout << "Dia chi     : " << u.diaChi << "\n";
}

// ======================= Ham lam viec voi DB qua ODBC =======================
bool chenNguoiDungGoc(SQLHDBC hDbc, const UserRecord& u, long long& rawId) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!thanhCong(rc)) {
        return false;
    }

    const SQLCHAR sqlInsert[] =
        "INSERT INTO NguoiDung (ho_ten, so_dien_thoai, email, cccd, dia_chi) "
        "VALUES (?, ?, ?, ?, ?)";

    rc = SQLPrepareA(hStmt, (SQLCHAR*)sqlInsert, SQL_NTS);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Prepare INSERT NguoiDung that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    // Bind 5 tham so dau vao
    SQLLEN idInd = 0;
    SQLLEN ind = 0;
    rc = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)u.hoTen, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind ho_ten that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)u.soDienThoai, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind so_dien_thoai that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 120, 0, (SQLPOINTER)u.email, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind email that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)u.cccd, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind cccd that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)u.diaChi, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind dia_chi that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLExecute(hStmt);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Execute INSERT NguoiDung that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    // Lay ID vua insert: SELECT LAST_INSERT_ID()
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!thanhCong(rc)) {
        return false;
    }

    const SQLCHAR sqlLastId[] = "SELECT LAST_INSERT_ID()";
    rc = SQLExecDirectA(hStmt, (SQLCHAR*)sqlLastId, SQL_NTS);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "SELECT LAST_INSERT_ID that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLFetch(hStmt);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Fetch LAST_INSERT_ID that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLBIGINT id = 0;
    rc = SQLGetData(hStmt, 1, SQL_C_SBIGINT, &id, sizeof(id), &idInd);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay du lieu LAST_INSERT_ID that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rawId = (long long)id;
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;
}

bool chenGoiTinCongKhai(SQLHDBC hDbc, long long nguoiDungId, const UserRecord& m) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!thanhCong(rc)) {
        return false;
    }

    const SQLCHAR sqlInsert[] =
        "INSERT INTO GoiTinCongKhai "
        "(nguoi_dung_id, ho_ten_masked, so_dien_thoai_masked, email_masked, cccd_masked, dia_chi_masked) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    rc = SQLPrepareA(hStmt, (SQLCHAR*)sqlInsert, SQL_NTS);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Prepare INSERT GoiTinCongKhai that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLBIGINT id = (SQLBIGINT)nguoiDungId;
    SQLLEN idInd = 0;
    rc = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &id, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind nguoi_dung_id that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)m.hoTen, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind ho_ten_masked that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)m.soDienThoai, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind so_dien_thoai_masked that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 120, 0, (SQLPOINTER)m.email, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind email_masked that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)m.cccd, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind cccd_masked that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)m.diaChi, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind dia_chi_masked that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLExecute(hStmt);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Execute INSERT GoiTinCongKhai that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;
}

bool docVaInKetQua(SQLHDBC hDbc, long long nguoiDungId) {
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!thanhCong(rc)) {
        return false;
    }

    const SQLCHAR sqlSelect[] =
        "SELECT n.ho_ten, n.so_dien_thoai, n.email, n.cccd, n.dia_chi, "
        "g.ho_ten_masked, g.so_dien_thoai_masked, g.email_masked, g.cccd_masked, g.dia_chi_masked "
        "FROM NguoiDung n "
        "JOIN GoiTinCongKhai g ON n.id = g.nguoi_dung_id "
        "WHERE n.id = ?";

    rc = SQLPrepareA(hStmt, (SQLCHAR*)sqlSelect, SQL_NTS);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Prepare SELECT that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLBIGINT id = (SQLBIGINT)nguoiDungId;
    SQLLEN idInd = 0;
    rc = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &id, 0, NULL);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Bind id cho SELECT that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLExecute(hStmt);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Execute SELECT that bai");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    // Bo nho nhan du lieu tu DB
    char hoTen[MAX_HO_TEN] = {0};
    char sdt[MAX_SDT] = {0};
    char email[MAX_EMAIL] = {0};
    char cccd[MAX_CCCD] = {0};
    char diaChi[MAX_DIA_CHI] = {0};
    char hoTenM[MAX_HO_TEN] = {0};
    char sdtM[MAX_SDT] = {0};
    char emailM[MAX_EMAIL] = {0};
    char cccdM[MAX_CCCD] = {0};
    char diaChiM[MAX_DIA_CHI] = {0};
    SQLLEN ind = 0;
    rc = SQLFetch(hStmt);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Khong fetch duoc ket qua");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    rc = SQLGetData(hStmt, 1, SQL_C_CHAR, hoTen, MAX_HO_TEN, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot ho_ten that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 2, SQL_C_CHAR, sdt, MAX_SDT, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot so_dien_thoai that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 3, SQL_C_CHAR, email, MAX_EMAIL, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot email that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 4, SQL_C_CHAR, cccd, MAX_CCCD, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot cccd that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 5, SQL_C_CHAR, diaChi, MAX_DIA_CHI, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot dia_chi that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 6, SQL_C_CHAR, hoTenM, MAX_HO_TEN, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot ho_ten_masked that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 7, SQL_C_CHAR, sdtM, MAX_SDT, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot so_dien_thoai_masked that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 8, SQL_C_CHAR, emailM, MAX_EMAIL, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot email_masked that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 9, SQL_C_CHAR, cccdM, MAX_CCCD, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot cccd_masked that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    rc = SQLGetData(hStmt, 10, SQL_C_CHAR, diaChiM, MAX_DIA_CHI, &ind);
    if (!thanhCong(rc)) { inChanDoanODBC(SQL_HANDLE_STMT, hStmt, "Lay cot dia_chi_masked that bai"); SQLFreeHandle(SQL_HANDLE_STMT, hStmt); return false; }
    cout << "\n========== DU LIEU DOC TU DATABASE ==========\n";
    cout << "[BANG NguoiDung - Goc]\n";
    cout << "Ho ten       : " << hoTen << "\n";
    cout << "So dien thoai: " << sdt << "\n";
    cout << "Email        : " << email << "\n";
    cout << "CCCD         : " << cccd << "\n";
    cout << "Dia chi      : " << diaChi << "\n";

    cout << "\n[BANG GoiTinCongKhai - Da che]\n";
    cout << "Ho ten       : " << hoTenM << "\n";
    cout << "So dien thoai: " << sdtM << "\n";
    cout << "Email        : " << emailM << "\n";
    cout << "CCCD         : " << cccdM << "\n";
    cout << "Dia chi      : " << diaChiM << "\n";

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;
}

int main() {
    // Dat locale de hien thi chu Unicode tot hon trong mot so truong hop
    setlocale(LC_ALL, "");

    cout << "=== CHUONG TRINH DATA MASKING + MYSQL ODBC ===\n";
    cout << "Luu y: Hay tao DB/bang truoc theo script da cung cap.\n\n";

    UserRecord banGhiGoc;
    UserRecord banGhiMasked;

    // Nếu còn dữ liệu thừa trong bộ đệm, bỏ qua trước khi getline
    if (cin.peek() == '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // 1) Nhap du lieu nguoi dung
    nhapThongTinNguoiDung(banGhiGoc);

    // 2) Tao du lieu da che
    taoBanGhiDaMask(banGhiGoc, banGhiMasked);

    // 3) Hien thi ngay tren console de so sanh nhanh
    inBanGhi("Du lieu goc (bo nho)", banGhiGoc);
    inBanGhi("Du lieu da che (bo nho)", banGhiMasked);

    // 4) Khoi tao ODBC ENV + DBC
    SQLHENV hEnv = SQL_NULL_HENV;
    SQLHDBC hDbc = SQL_NULL_HDBC;
    SQLRETURN rc;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (!thanhCong(rc)) {
        cout << "Khong tao duoc ODBC ENV.\n";
        return 1;
    }

    rc = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_ENV, hEnv, "SQLSetEnvAttr that bai");
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }

    rc = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (!thanhCong(rc)) {
        cout << "Khong tao duoc ODBC DBC.\n";
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }

    // Ket noi bang DSN 64-bit da tao san trong ODBC Data Sources (64-bit)
    // Vi du DSN: CSATBMTT_DSN64
    SQLCHAR connStrIn[] =
        "DSN=CSATBMTT_DSN64;"
        "UID=masking_app;"
        "PWD=Masking@12345;";
    SQLCHAR connStrOut[1024];
    SQLSMALLINT outLen;

    rc = SQLDriverConnectA(
        hDbc,
        NULL,
        connStrIn,
        SQL_NTS,
        connStrOut,
        sizeof(connStrOut),
        &outLen,
        SQL_DRIVER_NOPROMPT
    );

    if (!thanhCong(rc)) {
        inChanDoanODBC(SQL_HANDLE_DBC, hDbc, "Ket noi MySQL ODBC that bai");
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }

    cout << "\nKet noi MySQL ODBC thanh cong.\n";

    // 5) Luu ban ghi goc
    long long nguoiDungId = 0;
    if (!chenNguoiDungGoc(hDbc, banGhiGoc, nguoiDungId)) {
        cout << "Luu du lieu goc that bai.\n";
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }
    cout << "Da luu NguoiDung, id = " << nguoiDungId << "\n";

    // 6) Luu ban ghi da che
    if (!chenGoiTinCongKhai(hDbc, nguoiDungId, banGhiMasked)) {
        cout << "Luu du lieu da che that bai.\n";
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }
    cout << "Da luu GoiTinCongKhai thanh cong.\n";

    // 7) Doc lai va hien thi tu DB
    if (!docVaInKetQua(hDbc, nguoiDungId)) {
        cout << "Doc va hien thi du lieu tu DB that bai.\n";
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return 1;
    }

    // 8) Dong ket noi
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    cout << "\nHoan tat chuong trinh.\n";
    return 0;
}
