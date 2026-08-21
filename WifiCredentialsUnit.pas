unit WifiCredentialsUnit;

interface

uses
  Winapi.Windows, Winapi.Messages,
  System.SysUtils, System.Classes, System.IniFiles,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.StdCtrls, Vcl.Dialogs;

function EncodeIniSecret(const S: string): string;
function DecodeIniSecret(const S: string): string;

type
  TfrmWifiCredentials = class(TForm)
    lblTitle: TLabel;
    lblSSID: TLabel;
    lblPassword: TLabel;
    edtSSID: TEdit;
    edtPassword: TEdit;
    btnOK: TButton;
    btnCancel: TButton;
    procedure FormCreate(Sender: TObject);
    procedure btnOKClick(Sender: TObject);
  private
    FIniFileName: string;
  public
    function Execute(const AIniFileName: string): Boolean;
  end;

implementation

const
  CIniKey: array[0..15] of Byte = (
    $4F, $48, $4D, $2D, $41, $54, $53, $2D,
    $4C, $41, $42, $2D, $32, $30, $32, $36
  );

function BytesToHex(const B: TBytes): string;
var
  I: Integer;
begin
  Result := '';
  for I := 0 to High(B) do
    Result := Result + IntToHex(B[I], 2);
end;

function HexToBytes(const S: string): TBytes;
var
  I, N: Integer;
begin
  if Odd(Length(S)) then
    raise Exception.Create('Valeur ATSENC1 invalide.');

  SetLength(Result, Length(S) div 2);
  N := 0;
  I := 1;

  while I <= Length(S) do
  begin
    Result[N] := StrToInt('$' + Copy(S, I, 2));
    Inc(N);
    Inc(I, 2);
  end;
end;

procedure CryptIniBytes(var B: TBytes);
var
  I: Integer;
  K: Byte;
begin
  for I := 0 to High(B) do
  begin
    { XOR avec cle tournante + position, identique pour coder/decoder }
    K := CIniKey[I mod Length(CIniKey)] xor Byte((I * 29 + 73) and $FF);
    B[I] := B[I] xor K;
  end;
end;

function EncodeIniSecret(const S: string): string;
var
  B: TBytes;
begin
  B := TEncoding.UTF8.GetBytes(S);
  CryptIniBytes(B);
  Result := 'ATSENC1:' + BytesToHex(B);
end;

function DecodeIniSecret(const S: string): string;
var
  Enc: string;
  B: TBytes;
begin
  { Ancien INI en clair : retourner tel quel pour migration automatique. }
  if Pos('ATSENC1:', UpperCase(S)) <> 1 then
  begin
    Result := S;
    Exit;
  end;

  Enc := Copy(S, 9, MaxInt);
  B := HexToBytes(Enc);
  CryptIniBytes(B);
  Result := TEncoding.UTF8.GetString(B);
end;


{$R *.dfm}

procedure TfrmWifiCredentials.FormCreate(Sender: TObject);
begin
  edtPassword.PasswordChar := '*';
end;

function TfrmWifiCredentials.Execute(
  const AIniFileName: string): Boolean;
var
  Ini: TIniFile;
  RawSSID: string;
  RawPassword: string;
  PlainSSID: string;
  PlainPassword: string;
begin
  FIniFileName := AIniFileName;

  Ini := TIniFile.Create(FIniFileName);
  try
    RawSSID := Ini.ReadString('WiFi', 'SSID', '');
    RawPassword := Ini.ReadString('WiFi', 'Password', '');

    PlainSSID := DecodeIniSecret(RawSSID);
    PlainPassword := DecodeIniSecret(RawPassword);

    edtSSID.Text := PlainSSID;
    edtPassword.Text := PlainPassword;

    { Migration automatique d'un ancien INI encore lisible. }
    if (RawSSID <> '') and
       (Pos('ATSENC1:', UpperCase(RawSSID)) <> 1) then
      Ini.WriteString('WiFi', 'SSID', EncodeIniSecret(PlainSSID));

    if (RawPassword <> '') and
       (Pos('ATSENC1:', UpperCase(RawPassword)) <> 1) then
      Ini.WriteString('WiFi', 'Password', EncodeIniSecret(PlainPassword));

    Ini.UpdateFile;
  finally
    Ini.Free;
  end;

  Result := ShowModal = mrOk;
end;


procedure TfrmWifiCredentials.btnOKClick(Sender: TObject);
var
  Ini: TIniFile;
begin
  if Trim(edtSSID.Text) = '' then
  begin
    MessageDlg(
      'Entre le SSID du reseau Wi-Fi.',
      mtWarning, [mbOK], 0
    );
    Exit;
  end;

  Ini := TIniFile.Create(FIniFileName);
  try
    Ini.WriteString(
      'WiFi',
      'SSID',
      EncodeIniSecret(Trim(edtSSID.Text))
    );

    Ini.WriteString(
      'WiFi',
      'Password',
      EncodeIniSecret(edtPassword.Text)
    );

    Ini.UpdateFile;
  finally
    Ini.Free;
  end;

  ModalResult := mrOk;
end;


end.
