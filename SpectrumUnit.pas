unit SpectrumUnit;

interface

uses
  Winapi.Windows,
  System.SysUtils, System.Classes, System.Math,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.ExtCtrls, Vcl.StdCtrls;

type
  TSpectrumStopEvent = procedure(Sender: TObject) of object;
  TSpectrumScanEndEvent = procedure(Sender: TObject) of object;

  TfrmSpectrum = class(TForm)
    pbSpectrum: TPaintBox;
    lblTitle: TLabel;
    lblRange: TLabel;
    lblPeak: TLabel;
    btnClose: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure pbSpectrumPaint(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
  private
    FRSSI: TArray<Integer>;
    FSNR: TArray<Integer>;
    FCount: Integer;
    FBaseKHz: Double;
    FStepKHz: Double;
    FReceiving: Boolean;
    FReceivedCount: Integer;
    FOnStopRequest: TSpectrumStopEvent;
    FOnScanEnd: TSpectrumScanEndEvent;
    procedure ClearData;
    procedure UpdateInfo;
    procedure UpdateProgress;
    function ValueAfterKey(const ALine, AKey: string): string;
  public
    procedure BeginScan;
    procedure AbortScan(const AReason: string);
    procedure ProcessScanLine(const ALine: string);
    property Receiving: Boolean read FReceiving;
    property OnStopRequest: TSpectrumStopEvent read FOnStopRequest write FOnStopRequest;
    property OnScanEnd: TSpectrumScanEndEvent read FOnScanEnd write FOnScanEnd;
  end;

var
  frmSpectrum: TfrmSpectrum;

implementation

const
  CMaxScanPoints = 4096;

{$R *.dfm}

procedure TfrmSpectrum.FormCreate(Sender: TObject);
begin
  DoubleBuffered := True;
  FCount := 0;
  FBaseKHz := 0;
  FStepKHz := 0;
  FReceiving := False;
  FReceivedCount := 0;
  lblRange.Caption := 'En attente des donnees du scanner ATS...';
  lblPeak.Caption := 'PIC : ---';
end;

procedure TfrmSpectrum.ClearData;
begin
  FCount := 0;
  SetLength(FRSSI, 0);
  SetLength(FSNR, 0);
  FBaseKHz := 0;
  FStepKHz := 0;
  FReceiving := False;
  FReceivedCount := 0;
  pbSpectrum.Invalidate;
end;

procedure TfrmSpectrum.AbortScan(const AReason: string);
begin
  FReceiving := False;
  if Trim(AReason) <> '' then
    lblRange.Caption := AReason;
  pbSpectrum.Invalidate;
end;

procedure TfrmSpectrum.BeginScan;
begin
  ClearData;
  lblRange.Caption := 'Initialisation du spectre...';
  lblPeak.Caption := 'PIC : ---';
  Show;
  BringToFront;
end;

procedure TfrmSpectrum.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caHide;
  if Assigned(FOnStopRequest) then
    FOnStopRequest(Self);
end;

procedure TfrmSpectrum.btnCloseClick(Sender: TObject);
begin
  Close;
end;

function TfrmSpectrum.ValueAfterKey(const ALine, AKey: string): string;
var
  P, E: Integer;
begin
  Result := '';
  P := Pos(UpperCase(AKey), UpperCase(ALine));
  if P = 0 then Exit;
  Inc(P, Length(AKey));
  E := P;
  while (E <= Length(ALine)) and (ALine[E] <> ',') do Inc(E);
  Result := Trim(Copy(ALine, P, E - P));
end;

procedure TfrmSpectrum.ProcessScanLine(const ALine: string);
var
  L: string;
  Parts: TArray<string>;
  StartIndex, I, DataIndex, R, S: Integer;
  FS: TFormatSettings;
begin
  L := Trim(ALine);
  FS := TFormatSettings.Invariant;

  if SameText(Copy(L, 1, 10), 'SCANBEGIN,') then
  begin
    FCount := StrToIntDef(ValueAfterKey(L, 'COUNT='), 0);
    FBaseKHz := StrToFloatDef(ValueAfterKey(L, 'BASE_KHZ='), 0, FS);
    FStepKHz := StrToFloatDef(ValueAfterKey(L, 'STEP_KHZ='), 0, FS);
    { Certains firmwares FM renvoient BASE_KHZ en dizaines de kHz :
      9594 représente alors 95940 kHz. STEP_KHZ reste déjà exprimé en kHz. }
    if (FBaseKHz >= 8750) and (FBaseKHz <= 10800) then
      FBaseKHz := FBaseKHz * 10;
    FCount := EnsureRange(FCount, 0, CMaxScanPoints);
    SetLength(FRSSI, FCount);
    SetLength(FSNR, FCount);
    if FCount > 0 then
    begin
      FillChar(FRSSI[0], FCount * SizeOf(Integer), 0);
      FillChar(FSNR[0], FCount * SizeOf(Integer), 0);
    end;
    FReceiving := True;
    FReceivedCount := 0;
    UpdateProgress;
    Exit;
  end;

  if SameText(Copy(L, 1, 9), 'SCANDATA,') then
  begin
    Parts := L.Split([',']);
    if Length(Parts) < 4 then Exit;
    StartIndex := StrToIntDef(Parts[1], 0);
    if StartIndex < 0 then
      StartIndex := 0;
    DataIndex := StartIndex;
    I := 2;
    while (I + 1 < Length(Parts)) and (DataIndex < FCount) do
    begin
      R := StrToIntDef(Trim(Parts[I]), 0);
      S := StrToIntDef(Trim(Parts[I + 1]), 0);
      FRSSI[DataIndex] := EnsureRange(R, 0, 100);
      FSNR[DataIndex] := EnsureRange(S, 0, 100);
      Inc(DataIndex);
      Inc(I, 2);
    end;
    if DataIndex > FReceivedCount then
      FReceivedCount := DataIndex;
    UpdateProgress;
    Exit;
  end;

  if SameText(L, 'SCANEND') then
  begin
    FReceiving := False;
    UpdateInfo;
    pbSpectrum.Invalidate;
    if Assigned(FOnScanEnd) then
      FOnScanEnd(Self);
  end;
end;

procedure TfrmSpectrum.UpdateProgress;
begin
  if FCount <= 0 then
    lblRange.Caption := 'Scan en cours : attente des points...'
  else
    lblRange.Caption := Format('Scan en cours : %d / %d points',
      [EnsureRange(FReceivedCount, 0, FCount), FCount]);
end;

procedure TfrmSpectrum.UpdateInfo;
var
  I, PeakValue, PeakIndex: Integer;
  EndKHz, PeakKHz: Double;
begin
  if (FCount <= 0) or (FStepKHz <= 0) then
  begin
    lblRange.Caption := 'Aucune donnee de spectre';
    lblPeak.Caption := 'PIC : ---';
    Exit;
  end;

  EndKHz := FBaseKHz + ((FCount - 1) * FStepKHz);
  lblRange.Caption := Format('%.3f MHz  -  %.3f MHz    Pas %.3f kHz',
    [FBaseKHz / 1000, EndKHz / 1000, FStepKHz]);

  PeakValue := -1;
  PeakIndex := 0;
  for I := 0 to FCount - 1 do
    if FRSSI[I] > PeakValue then
    begin
      PeakValue := FRSSI[I];
      PeakIndex := I;
    end;

  PeakKHz := FBaseKHz + (PeakIndex * FStepKHz);
  lblPeak.Caption := Format('PIC : %.3f MHz   RSSI %d   SNR %d',
    [PeakKHz / 1000, PeakValue, FSNR[PeakIndex]]);
end;

procedure TfrmSpectrum.pbSpectrumPaint(Sender: TObject);
const
  MarginL = 54;
  MarginR = 18;
  MarginT = 18;
  MarginB = 38;
var
  C: TCanvas;
  R: TRect;
  PlotW, PlotH, I, X, Y, PrevX, PrevY, Grid: Integer;
  MaxRSSI, ScaleMax: Integer;
  FreqKHz: Double;
  S: string;
begin
  C := pbSpectrum.Canvas;
  R := pbSpectrum.ClientRect;
  C.Brush.Color := RGB(14, 18, 17);
  C.FillRect(R);

  PlotW := Max(1, R.Width - MarginL - MarginR);
  PlotH := Max(1, R.Height - MarginT - MarginB);

  C.Pen.Color := RGB(68, 78, 72);
  C.Font.Name := 'Consolas';
  C.Font.Size := 8;
  C.Font.Color := RGB(194, 176, 115);

  MaxRSSI := 0;
  for I := 0 to FCount - 1 do
    if FRSSI[I] > MaxRSSI then
      MaxRSSI := FRSSI[I];
  ScaleMax := EnsureRange(((MaxRSSI + 9) div 10) * 10, 20, 100);

  for Grid := 0 to 5 do
  begin
    Y := MarginT + MulDiv(Grid, PlotH, 5);
    C.MoveTo(MarginL, Y);
    C.LineTo(MarginL + PlotW, Y);
    S := IntToStr(ScaleMax - MulDiv(Grid, ScaleMax, 5));
    C.TextOut(8, Y - 7, S);
  end;

  for Grid := 0 to 4 do
  begin
    X := MarginL + MulDiv(Grid, PlotW, 4);
    C.MoveTo(X, MarginT);
    C.LineTo(X, MarginT + PlotH);
    if (FCount > 0) and (FStepKHz > 0) then
    begin
      FreqKHz := FBaseKHz + ((FCount - 1) * FStepKHz * Grid / 4);
      S := Format('%.3f', [FreqKHz / 1000]);
      C.TextOut(X - 25, MarginT + PlotH + 8, S);
    end;
  end;

  C.Pen.Color := RGB(201, 172, 84);
  C.Rectangle(MarginL, MarginT, MarginL + PlotW, MarginT + PlotH);

  if FCount < 2 then Exit;

  C.Pen.Color := RGB(75, 230, 125);
  C.Pen.Width := 2;
  PrevX := MarginL;
  PrevY := MarginT + PlotH -
    MulDiv(EnsureRange(FRSSI[0], 0, ScaleMax), PlotH, ScaleMax);
  for I := 1 to FCount - 1 do
  begin
    X := MarginL + MulDiv(I, PlotW, FCount - 1);
    Y := MarginT + PlotH -
      MulDiv(EnsureRange(FRSSI[I], 0, ScaleMax), PlotH, ScaleMax);
    C.MoveTo(PrevX, PrevY);
    C.LineTo(X, Y);
    PrevX := X;
    PrevY := Y;
  end;
  C.Pen.Width := 1;
end;

end.
