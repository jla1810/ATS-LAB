unit SerialMonitorUnit;

interface

uses
  Winapi.Windows, Winapi.Messages,
  System.SysUtils, System.Classes,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.StdCtrls, Vcl.ExtCtrls;

type
  TfrmSerialMonitor = class(TForm)
    pnlTop: TPanel;
    lblTitle: TLabel;
    btnClear: TButton;
    memSerial: TMemo;
    procedure FormCreate(Sender: TObject);
    procedure btnClearClick(Sender: TObject);
  public
    procedure AddLine(const AText: string);
  end;

implementation

{$R *.dfm}

procedure TfrmSerialMonitor.FormCreate(Sender: TObject);
begin
  DoubleBuffered := True;
  memSerial.Clear;
  AddLine('--- SUIVI SERIE ATS LAB ---');
end;

procedure TfrmSerialMonitor.btnClearClick(Sender: TObject);
begin
  memSerial.Clear;
end;

procedure TfrmSerialMonitor.AddLine(const AText: string);
begin
  if not HandleAllocated then
    Exit;

  memSerial.Lines.Add(
    FormatDateTime('hh:nn:ss.zzz', Now) + '  ' + AText
  );

  memSerial.SelStart := Length(memSerial.Text);
  memSerial.Perform(EM_SCROLLCARET, 0, 0);
end;

end.
