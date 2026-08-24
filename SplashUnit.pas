unit SplashUnit;

interface

uses
  Winapi.Windows,
  System.Classes,
  System.SysUtils,
  Vcl.Controls,
  Vcl.ExtCtrls,
  Vcl.Forms,
  Vcl.Graphics,
  Vcl.StdCtrls,
  Vcl.Imaging.pngimage;

type
  TfrmSplash = class(TForm)
  private
    FBackground: TImage;
    procedure AddCenteredLabel(const ACaption: string; ATop, AHeight,
      AFontSize: Integer; AColor: TColor; const AFontName: string);
  public
    constructor CreateNew(AOwner: TComponent; Dummy: Integer = 0); override;
  end;

implementation

constructor TfrmSplash.CreateNew(AOwner: TComponent; Dummy: Integer);
var
  ImagePath: string;
begin
  inherited CreateNew(AOwner, Dummy);
  BorderStyle := bsNone;
  ClientWidth := 768;
  ClientHeight := 512;
  Color := RGB(8, 7, 5);
  DoubleBuffered := True;
  FormStyle := fsStayOnTop;
  Position := poScreenCenter;

  FBackground := TImage.Create(Self);
  FBackground.Parent := Self;
  FBackground.Align := alClient;
  FBackground.Stretch := True;
  FBackground.Proportional := False;

  ImagePath := IncludeTrailingPathDelimiter(ExtractFilePath(Application.ExeName)) +
    'Data\Backgrounds\ats_lab_splash.png';
  if FileExists(ImagePath) then
    FBackground.Picture.LoadFromFile(ImagePath);

  AddCenteredLabel('ATS LAB', 145, 64, 38, RGB(239, 190, 87), 'Georgia');
  AddCenteredLabel('EXPERIMENTAL WIRELESS RECEIVER', 215, 34, 15,
    RGB(214, 158, 66), 'Georgia');
  AddCenteredLabel('ATS-25X2  /  SI4735', 258, 30, 13,
    RGB(184, 204, 144), 'Consolas');
  AddCenteredLabel('USB   |   BLUETOOTH SPP   |   WI-FI TCP', 300, 28, 11,
    RGB(121, 218, 151), 'Consolas');
  AddCenteredLabel('Version 1.1.1', 344, 28, 12,
    RGB(239, 190, 87), 'Georgia');
  AddCenteredLabel('INITIALISATION DES INSTRUMENTS...', 392, 24, 9,
    RGB(171, 145, 88), 'Consolas');
end;

procedure TfrmSplash.AddCenteredLabel(const ACaption: string;
  ATop, AHeight, AFontSize: Integer; AColor: TColor;
  const AFontName: string);
var
  L: TLabel;
begin
  L := TLabel.Create(Self);
  L.Parent := Self;
  L.SetBounds(92, ATop, ClientWidth - 184, AHeight);
  L.Alignment := taCenter;
  L.AutoSize := False;
  L.Caption := ACaption;
  L.Font.Name := AFontName;
  L.Font.Size := AFontSize;
  L.Font.Color := AColor;
  L.Font.Style := [fsBold];
  L.Layout := tlCenter;
  L.Transparent := True;
  L.BringToFront;
end;

end.
