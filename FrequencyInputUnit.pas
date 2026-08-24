unit FrequencyInputUnit;

interface

uses
  System.SysUtils, System.Classes,
  Vcl.Forms, Vcl.StdCtrls, Vcl.Dialogs, Vcl.Controls, Vcl.ExtCtrls,
  Vcl.Imaging.pngimage;

type
  TfrmFrequencyInput = class(TForm)
    imgBackground: TImage;
    lblTitle: TLabel;
    lblHint: TLabel;
    edtFrequency: TEdit;
    btnOK: TButton;
    btnCancel: TButton;
    procedure btnOKClick(Sender: TObject);
  private
    FFrequencyHz: Int64;
  public
    function Execute(const ACurrentHz: Int64): Boolean;
    property FrequencyHz: Int64 read FFrequencyHz;
  end;

implementation

{$R *.dfm}

function TfrmFrequencyInput.Execute(const ACurrentHz: Int64): Boolean;
begin
  if ACurrentHz >= 1000000 then
    edtFrequency.Text := FormatFloat('0.000000', ACurrentHz / 1000000.0)
  else
    edtFrequency.Text := FormatFloat('0.000', ACurrentHz / 1000.0);
  edtFrequency.SelectAll;
  ActiveControl := edtFrequency;
  Result := ShowModal = mrOk;
end;

procedure TfrmFrequencyInput.btnOKClick(Sender: TObject);
var
  S: string;
  V: Double;
begin
  S := Trim(edtFrequency.Text);
  S := StringReplace(S, '.', FormatSettings.DecimalSeparator, [rfReplaceAll]);
  S := StringReplace(S, ',', FormatSettings.DecimalSeparator, [rfReplaceAll]);

  if not TryStrToFloat(S, V) then
  begin
    MessageDlg('Frequence incorrecte.', mtWarning, [mbOK], 0);
    Exit;
  end;

  if V < 1000 then
    FFrequencyHz := Round(V * 1000000.0)
  else
    FFrequencyHz := Round(V * 1000.0);

  if (FFrequencyHz < 100000) or (FFrequencyHz > 108000000) then
  begin
    MessageDlg('Frequence hors plage.', mtWarning, [mbOK], 0);
    Exit;
  end;

  ModalResult := mrOk;
end;

end.
