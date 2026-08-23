unit SpectrumUnit;

interface

uses
  Winapi.Windows,
  System.SysUtils, System.Classes, System.Math, System.Generics.Collections,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.ExtCtrls, Vcl.StdCtrls,
  Vcl.Dialogs;

type
  TSpectrumStopEvent = procedure(Sender: TObject) of object;
  TSpectrumScanEndEvent = procedure(Sender: TObject) of object;
  TSpectrumTuneEvent = procedure(Sender: TObject;
    AFrequencyKHz: Int64) of object;
  TSpectrumStartEvent = procedure(Sender: TObject;
    AStartMHz, AStepKHz: Double) of object;

  TfrmSpectrum = class(TForm)
    pbSpectrum: TPaintBox;
    lblTitle: TLabel;
    lblRange: TLabel;
    lblPeak: TLabel;
    btnClose: TButton;
    lblStartFrequency: TLabel;
    edtStartFrequency: TEdit;
    lblStartUnit: TLabel;
    lblStep: TLabel;
    edtStep: TEdit;
    lblStepUnit: TLabel;
    btnStart: TButton;
    btnPause: TButton;
    btnResume: TButton;
    btnStop: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure pbSpectrumPaint(Sender: TObject);
    procedure pbSpectrumMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnCloseClick(Sender: TObject);
    procedure btnStartClick(Sender: TObject);
    procedure btnPauseClick(Sender: TObject);
    procedure btnResumeClick(Sender: TObject);
    procedure btnStopClick(Sender: TObject);
  private
    FRSSI: TArray<Integer>;
    FSNR: TArray<Integer>;
    FCount: Integer;
    FBaseKHz: Double;
    FStepKHz: Double;
    FReceiving: Boolean;
    FReceivedCount: Integer;
    FSelectedIndex: Integer;
    FPeakIndices: TArray<Integer>;
    FNoiseFloor: Integer;
    FPeakThreshold: Integer;
    FOnStopRequest: TSpectrumStopEvent;
    FOnScanEnd: TSpectrumScanEndEvent;
    FOnTuneRequest: TSpectrumTuneEvent;
    FOnStartRequest: TSpectrumStartEvent;
    FOnPauseRequest: TSpectrumStopEvent;
    FOnResumeRequest: TSpectrumStopEvent;
    procedure ClearData;
    procedure UpdateInfo;
    procedure UpdateProgress;
    procedure DetectPeaks;
    function ValueAfterKey(const ALine, AKey: string): string;
  public
    procedure BeginScan;
    procedure AbortScan(const AReason: string);
    procedure ProcessScanLine(const ALine: string);
    procedure PrepareScan(const AStartMHz, AStepKHz: Double);
    procedure SetPaused(const APaused: Boolean);
    property Receiving: Boolean read FReceiving;
    property OnStopRequest: TSpectrumStopEvent read FOnStopRequest write FOnStopRequest;
    property OnScanEnd: TSpectrumScanEndEvent read FOnScanEnd write FOnScanEnd;
    property OnTuneRequest: TSpectrumTuneEvent read FOnTuneRequest write FOnTuneRequest;
    property OnStartRequest: TSpectrumStartEvent read FOnStartRequest write FOnStartRequest;
    property OnPauseRequest: TSpectrumStopEvent read FOnPauseRequest write FOnPauseRequest;
    property OnResumeRequest: TSpectrumStopEvent read FOnResumeRequest write FOnResumeRequest;
  end;

var
  frmSpectrum: TfrmSpectrum;

implementation

const
  CMaxScanPoints = 4096;
  CSpectrumMarginL = 54;
  CSpectrumMarginR = 18;
  CSpectrumMarginT = 18;
  CSpectrumMarginB = 38;

{$R *.dfm}

procedure TfrmSpectrum.FormCreate(Sender: TObject);
begin
  DoubleBuffered := True;
  FCount := 0;
  FBaseKHz := 0;
  FStepKHz := 0;
  FReceiving := False;
  FReceivedCount := 0;
  FSelectedIndex := -1;
  FNoiseFloor := 0;
  FPeakThreshold := 6;
  SetLength(FPeakIndices, 0);
  pbSpectrum.OnMouseDown := pbSpectrumMouseDown;
  pbSpectrum.Hint := 'Clic gauche : accord   Clic droit : seuil des marqueurs';
  pbSpectrum.ShowHint := True;
  lblRange.Caption := 'En attente des donnees du scanner ATS...';
  lblPeak.Caption := 'PIC : ---';
  btnPause.Enabled := False;
  btnResume.Enabled := False;
  btnStop.Enabled := False;
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
  FSelectedIndex := -1;
  FNoiseFloor := 0;
  SetLength(FPeakIndices, 0);
  pbSpectrum.Invalidate;
end;

procedure TfrmSpectrum.AbortScan(const AReason: string);
begin
  FReceiving := False;
  btnStart.Enabled := True;
  btnPause.Enabled := False;
  btnResume.Enabled := False;
  btnStop.Enabled := False;
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
  btnStart.Enabled := False;
  btnPause.Enabled := True;
  btnResume.Enabled := False;
  btnStop.Enabled := True;
end;

procedure TfrmSpectrum.PrepareScan(const AStartMHz, AStepKHz: Double);
var
  FS: TFormatSettings;
begin
  FS := TFormatSettings.Create;
  edtStartFrequency.Text := FormatFloat('0.000', AStartMHz, FS);
  edtStep.Text := FormatFloat('0.###', AStepKHz, FS);
  btnStart.Enabled := True;
  btnPause.Enabled := False;
  btnResume.Enabled := False;
  btnStop.Enabled := False;
  Show;
  BringToFront;
end;

procedure TfrmSpectrum.SetPaused(const APaused: Boolean);
begin
  btnPause.Enabled := not APaused;
  btnResume.Enabled := APaused;
  btnStop.Enabled := True;
  if APaused then
    lblRange.Caption := 'Scan en pause'
  else
    lblRange.Caption := 'Reprise du scan...';
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

procedure TfrmSpectrum.btnStartClick(Sender: TObject);
var
  StartMHz, StepKHz: Double;
  StartText, StepText: string;
  FS: TFormatSettings;
begin
  FS := TFormatSettings.Create;
  StartText := StringReplace(Trim(edtStartFrequency.Text), '.',
    FS.DecimalSeparator, [rfReplaceAll]);
  StepText := StringReplace(Trim(edtStep.Text), '.',
    FS.DecimalSeparator, [rfReplaceAll]);
  if not TryStrToFloat(StartText, StartMHz, FS) or
     not TryStrToFloat(StepText, StepKHz, FS) then
  begin
    MessageDlg('Fréquence de départ ou pas invalide.', mtError, [mbOK], 0);
    Exit;
  end;
  if Assigned(FOnStartRequest) then
    FOnStartRequest(Self, StartMHz, StepKHz);
end;

procedure TfrmSpectrum.btnPauseClick(Sender: TObject);
begin
  if Assigned(FOnPauseRequest) then
    FOnPauseRequest(Self);
end;

procedure TfrmSpectrum.btnResumeClick(Sender: TObject);
begin
  if Assigned(FOnResumeRequest) then
    FOnResumeRequest(Self);
end;

procedure TfrmSpectrum.btnStopClick(Sender: TObject);
begin
  if Assigned(FOnStopRequest) then
    FOnStopRequest(Self);
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
    if not TryStrToInt(Trim(Parts[1]), StartIndex) or
       (StartIndex < 0) or (StartIndex >= FCount) then
      Exit;
    DataIndex := StartIndex;
    I := 2;
    while (I + 1 < Length(Parts)) and (DataIndex < FCount) do
    begin
      if not TryStrToInt(Trim(Parts[I]), R) or
         not TryStrToInt(Trim(Parts[I + 1]), S) then
      begin
        Inc(DataIndex);
        Inc(I, 2);
        Continue;
      end;
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
    btnStart.Enabled := True;
    btnPause.Enabled := False;
    btnResume.Enabled := False;
    btnStop.Enabled := False;
    DetectPeaks;
    UpdateInfo;
    pbSpectrum.Invalidate;
    if Assigned(FOnScanEnd) then
      FOnScanEnd(Self);
  end;
end;

procedure TfrmSpectrum.DetectPeaks;
const
  CMaxPeakMarkers = 16;
  CMinPeakDistance = 8;
var
  SortedRSSI: TArray<Integer>;
  I, PeakCount, LastIndex: Integer;
begin
  SetLength(FPeakIndices, 0);
  FNoiseFloor := 0;
  if FCount < 3 then
    Exit;

  SortedRSSI := Copy(FRSSI);
  TArray.Sort<Integer>(SortedRSSI);
  FNoiseFloor := SortedRSSI[FCount div 2];

  PeakCount := 0;
  LastIndex := -CMinPeakDistance;
  for I := 1 to FCount - 2 do
  begin
    if (FRSSI[I] < FNoiseFloor + FPeakThreshold) or
       (FRSSI[I] < FRSSI[I - 1]) or
       (FRSSI[I] <= FRSSI[I + 1]) then
      Continue;

    if (PeakCount > 0) and (I - LastIndex < CMinPeakDistance) then
    begin
      if FRSSI[I] > FRSSI[LastIndex] then
      begin
        FPeakIndices[PeakCount - 1] := I;
        LastIndex := I;
      end;
      Continue;
    end;

    SetLength(FPeakIndices, PeakCount + 1);
    FPeakIndices[PeakCount] := I;
    Inc(PeakCount);
    LastIndex := I;
    if PeakCount >= CMaxPeakMarkers then
      Break;
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
  lblRange.Caption := Format(
    '%.3f MHz  -  %.3f MHz    Pas %.3f kHz    Signaux %d (bruit %d, seuil +%d)',
    [FBaseKHz / 1000, EndKHz / 1000, FStepKHz,
     Length(FPeakIndices), FNoiseFloor, FPeakThreshold]);

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

procedure TfrmSpectrum.pbSpectrumMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  PlotW, PlotH, I, MarkerX, ClickedIndex, NewThreshold: Integer;
  FrequencyKHz: Int64;
  ThresholdText: string;
begin
  if FReceiving or (FCount < 2) or (FStepKHz <= 0) then
    Exit;

  if Button = mbRight then
  begin
    ThresholdText := IntToStr(FPeakThreshold);
    if InputQuery('SEUIL DE DETECTION',
      'RSSI au-dessus du bruit (1 a 30) :', ThresholdText) and
       TryStrToInt(Trim(ThresholdText), NewThreshold) and
       (NewThreshold >= 1) and (NewThreshold <= 30) then
    begin
      FPeakThreshold := NewThreshold;
      DetectPeaks;
      UpdateInfo;
      pbSpectrum.Invalidate;
    end;
    Exit;
  end;
  if Button <> mbLeft then
    Exit;

  PlotW := Max(1, pbSpectrum.ClientWidth -
    CSpectrumMarginL - CSpectrumMarginR);
  PlotH := Max(1, pbSpectrum.ClientHeight -
    CSpectrumMarginT - CSpectrumMarginB);
  if (X < CSpectrumMarginL) or
     (X > CSpectrumMarginL + PlotW) or
     (Y < CSpectrumMarginT) or
     (Y > CSpectrumMarginT + PlotH) then
    Exit;

  ClickedIndex := EnsureRange(
    Round((X - CSpectrumMarginL) * (FCount - 1) / PlotW),
    0, FCount - 1);
  for I := 0 to High(FPeakIndices) do
  begin
    MarkerX := CSpectrumMarginL +
      MulDiv(FPeakIndices[I], PlotW, FCount - 1);
    if Abs(X - MarkerX) <= 6 then
    begin
      ClickedIndex := FPeakIndices[I];
      Break;
    end;
  end;
  FSelectedIndex := ClickedIndex;
  FrequencyKHz := Round(FBaseKHz + (FSelectedIndex * FStepKHz));
  lblPeak.Caption := Format(
    'SELECTION : %.3f MHz   RSSI %d   SNR %d',
    [FrequencyKHz / 1000, FRSSI[FSelectedIndex], FSNR[FSelectedIndex]]);
  pbSpectrum.Invalidate;

  if Assigned(FOnTuneRequest) then
    FOnTuneRequest(Self, FrequencyKHz);
end;

procedure TfrmSpectrum.pbSpectrumPaint(Sender: TObject);
var
  C: TCanvas;
  R: TRect;
  PlotW, PlotH, I, X, Y, PrevX, PrevY, Grid, PeakIndex: Integer;
  MaxRSSI, ScaleMax: Integer;
  FreqKHz: Double;
  S: string;
begin
  C := pbSpectrum.Canvas;
  R := pbSpectrum.ClientRect;
  C.Brush.Style := bsSolid;
  C.Brush.Color := RGB(14, 18, 17);
  C.FillRect(R);

  PlotW := Max(1, R.Width - CSpectrumMarginL - CSpectrumMarginR);
  PlotH := Max(1, R.Height - CSpectrumMarginT - CSpectrumMarginB);

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
    Y := CSpectrumMarginT + MulDiv(Grid, PlotH, 5);
    C.MoveTo(CSpectrumMarginL, Y);
    C.LineTo(CSpectrumMarginL + PlotW, Y);
    S := IntToStr(ScaleMax - MulDiv(Grid, ScaleMax, 5));
    C.TextOut(8, Y - 7, S);
  end;

  for Grid := 0 to 4 do
  begin
    X := CSpectrumMarginL + MulDiv(Grid, PlotW, 4);
    C.MoveTo(X, CSpectrumMarginT);
    C.LineTo(X, CSpectrumMarginT + PlotH);
    if (FCount > 0) and (FStepKHz > 0) then
    begin
      FreqKHz := FBaseKHz + ((FCount - 1) * FStepKHz * Grid / 4);
      S := Format('%.3f', [FreqKHz / 1000]);
      C.TextOut(X - 25, CSpectrumMarginT + PlotH + 8, S);
    end;
  end;

  C.Pen.Color := RGB(201, 172, 84);
  C.Rectangle(CSpectrumMarginL, CSpectrumMarginT,
    CSpectrumMarginL + PlotW, CSpectrumMarginT + PlotH);

  if FCount < 2 then Exit;

  C.Pen.Color := RGB(75, 230, 125);
  C.Pen.Width := 2;
  PrevX := CSpectrumMarginL;
  PrevY := CSpectrumMarginT + PlotH -
    MulDiv(EnsureRange(FRSSI[0], 0, ScaleMax), PlotH, ScaleMax);
  for I := 1 to FCount - 1 do
  begin
    X := CSpectrumMarginL + MulDiv(I, PlotW, FCount - 1);
    Y := CSpectrumMarginT + PlotH -
      MulDiv(EnsureRange(FRSSI[I], 0, ScaleMax), PlotH, ScaleMax);
    C.MoveTo(PrevX, PrevY);
    C.LineTo(X, Y);
    PrevX := X;
    PrevY := Y;
  end;
  C.Pen.Width := 1;

  C.Pen.Color := RGB(255, 150, 35);
  C.Brush.Color := RGB(255, 150, 35);
  for I := 0 to High(FPeakIndices) do
  begin
    PeakIndex := FPeakIndices[I];
    if (PeakIndex < 0) or (PeakIndex >= FCount) then
      Continue;
    X := CSpectrumMarginL + MulDiv(PeakIndex, PlotW, FCount - 1);
    Y := CSpectrumMarginT + PlotH -
      MulDiv(EnsureRange(FRSSI[PeakIndex], 0, ScaleMax), PlotH, ScaleMax);
    C.Polygon([Point(X - 4, Y - 9), Point(X + 4, Y - 9), Point(X, Y - 2)]);
  end;
  C.Brush.Style := bsClear;

  if (FSelectedIndex >= 0) and (FSelectedIndex < FCount) then
  begin
    X := CSpectrumMarginL +
      MulDiv(FSelectedIndex, PlotW, FCount - 1);
    C.Pen.Color := RGB(255, 210, 70);
    C.MoveTo(X, CSpectrumMarginT);
    C.LineTo(X, CSpectrumMarginT + PlotH);
  end;
end;

end.
