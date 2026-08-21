unit AboutUnit;

interface

uses
  Winapi.Windows,
  Winapi.ShellAPI,
  System.SysUtils,
  System.Classes,
  Vcl.Forms,
  Vcl.Controls,
  Vcl.StdCtrls,
  Vcl.ExtCtrls,
  Vcl.Graphics;

type
  TAboutForm = class(TForm)
    imgBackground: TImage;
    lblTitle: TLabel;
    lblVersion: TLabel;
    lblProductTitle: TLabel;
    lblProduct: TLabel;
    lblConnectionTitle: TLabel;
    lblConnection: TLabel;
    lblDetailTitle: TLabel;
    lblDetail: TLabel;
    lblFirmwareTitle: TLabel;
    lblFirmware: TLabel;
    lblAuthorTitle: TLabel;
    lblAuthor: TLabel;
    lblDisclaimer: TLabel;
    btnGitHub: TButton;
    btnClose: TButton;
    procedure FormCreate(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure btnGitHubClick(Sender: TObject);
  private
    FGitHubURL: string;
    procedure ApplyTheme;
  public
    class procedure Execute(const AProgramName, AVersion, AProduct,
      AConnectionType, AConnectionDetail, AFirmware, AAuthor,
      AGitHubURL: string); static;
  end;

var
  AboutForm: TAboutForm;

implementation

{$R *.dfm}

class procedure TAboutForm.Execute(const AProgramName, AVersion, AProduct,
  AConnectionType, AConnectionDetail, AFirmware, AAuthor,
  AGitHubURL: string);
var
  F: TAboutForm;
begin
  F := TAboutForm.Create(nil);
  try
    F.lblTitle.Caption := AProgramName;
    F.lblVersion.Caption := 'Version ' + AVersion;
    F.lblProduct.Caption := AProduct;
    F.lblConnection.Caption := AConnectionType;
    F.lblDetail.Caption := AConnectionDetail;
    F.lblFirmware.Caption := AFirmware;
    F.lblAuthor.Caption := AAuthor;
    F.FGitHubURL := AGitHubURL;
    F.ShowModal;
  finally
    F.Free;
  end;
end;

procedure TAboutForm.FormCreate(Sender: TObject);
begin
  ApplyTheme;
end;

procedure TAboutForm.ApplyTheme;
begin
  { Meme palette que SerialConnect }
  Color := clBlack;

  lblTitle.Font.Color := clSilver;
  lblVersion.Font.Color := $0080FFFF;

  lblProductTitle.Font.Color := $00A6A6A6;
  lblConnectionTitle.Font.Color := $00A6A6A6;
  lblDetailTitle.Font.Color := $00A6A6A6;
  lblFirmwareTitle.Font.Color := $00A6A6A6;
  lblAuthorTitle.Font.Color := $00A6A6A6;

  lblProduct.Font.Color := $00DDDDDD;
  lblConnection.Font.Color := clLime;
  lblDetail.Font.Color := $00DDDDDD;
  lblFirmware.Font.Color := $00DDDDDD;
  lblAuthor.Font.Color := $0080FFFF;
  lblDisclaimer.Font.Color := clSilver;
end;

procedure TAboutForm.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrClose;
end;

procedure TAboutForm.btnGitHubClick(Sender: TObject);
begin
  if Trim(FGitHubURL) <> '' then
    ShellExecute(Handle, 'open', PChar(FGitHubURL), nil, nil, SW_SHOWNORMAL);
end;

end.
