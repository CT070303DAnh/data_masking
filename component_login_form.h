#pragma once

ref class LoginForm : public Form {
private:
    TextBox^ txtTenDangNhap;
    TextBox^ txtMatKhau;
    Button^ btnDangNhap;
    Button^ btnThoat;

public:
    Int64 IdNguoiDungDangNhap;
    String^ VaiTroDangNhap;

    LoginForm() {
        IdNguoiDungDangNhap = 0;
        VaiTroDangNhap = nullptr;
        this->Text = "Dang nhap";
        this->Size = Drawing::Size(420, 230);
        this->StartPosition = FormStartPosition::CenterScreen;
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
        this->MaximizeBox = false;
        this->MinimizeBox = false;

        Label^ lbTitle = gcnew Label();
        lbTitle->Text = "Vui long dang nhap de vao he thong";
        lbTitle->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
        lbTitle->Location = Point(20, 20);
        lbTitle->AutoSize = true;
        this->Controls->Add(lbTitle);

        Label^ lbUser = gcnew Label();
        lbUser->Text = "Tai khoan";
        lbUser->Location = Point(20, 65);
        lbUser->Size = Drawing::Size(90, 25);
        this->Controls->Add(lbUser);

        txtTenDangNhap = gcnew TextBox();
        txtTenDangNhap->Location = Point(120, 60);
        txtTenDangNhap->Size = Drawing::Size(250, 25);
        txtTenDangNhap->Text = "admin";
        this->Controls->Add(txtTenDangNhap);

        Label^ lbPass = gcnew Label();
        lbPass->Text = "Mat khau";
        lbPass->Location = Point(20, 100);
        lbPass->Size = Drawing::Size(90, 25);
        this->Controls->Add(lbPass);

        txtMatKhau = gcnew TextBox();
        txtMatKhau->Location = Point(120, 95);
        txtMatKhau->Size = Drawing::Size(250, 25);
        txtMatKhau->UseSystemPasswordChar = true;
        this->Controls->Add(txtMatKhau);

        btnDangNhap = gcnew Button();
        btnDangNhap->Text = "Dang nhap";
        btnDangNhap->Location = Point(120, 140);
        btnDangNhap->Size = Drawing::Size(110, 32);
        btnDangNhap->Click += gcnew EventHandler(this, &LoginForm::OnDangNhapClick);
        this->Controls->Add(btnDangNhap);

        btnThoat = gcnew Button();
        btnThoat->Text = "Thoat";
        btnThoat->Location = Point(260, 140);
        btnThoat->Size = Drawing::Size(110, 32);
        btnThoat->Click += gcnew EventHandler(this, &LoginForm::OnThoatClick);
        this->Controls->Add(btnThoat);

        this->AcceptButton = btnDangNhap;
        this->CancelButton = btnThoat;
    }

private:
    void OnDangNhapClick(Object^ sender, EventArgs^ e) {
        String^ thongBao = "";
        Int64 uid = 0;
        String^ vt = nullptr;
        if (xacThucTaiKhoanNguoiDung(txtTenDangNhap->Text, txtMatKhau->Text, thongBao, uid, vt)) {
            IdNguoiDungDangNhap = uid;
            VaiTroDangNhap = vt;
            this->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->Close();
            return;
        }

        MessageBox::Show(thongBao, "Dang nhap that bai", MessageBoxButtons::OK, MessageBoxIcon::Warning);
        txtMatKhau->Focus();
        txtMatKhau->SelectAll();
    }

    void OnThoatClick(Object^ sender, EventArgs^ e) {
        this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
        this->Close();
    }
};
