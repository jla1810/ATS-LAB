unit MainUnit;

interface

uses
  Winapi.Windows, Winapi.Messages,
  System.SysUtils, System.StrUtils, System.Classes, System.Math, System.IOUtils, System.IniFiles,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.ExtCtrls, Vcl.StdCtrls,
  Vcl.Imaging.pngimage,
  uGraphicsCache,
  uSteamVisuals,
  uRotatingNeedle,
  uSteamKnob,
  uSteamButton,
  uSteamButtonManager,
  SerialMonitorUnit,
  SerialConnectUnit, FrequencyInputUnit,
  WifiCredentialsUnit, AboutUnit, SpectrumUnit, FavoritesUnit,
  uATSConnection,
  uATSProtocol, Vcl.ActnMan, Vcl.ActnColorMaps,
  Vcl.Dialogs, System.UITypes;

type
  THamBandIndex = (
    hb160m, hb80m, hb40m, hb20m, hb17m,
    hb15m, hb12m, hb10m, hbCB
  );

  THamBandMemory = record
    FrequencyHz: Int64;
    Mode: string;
    Initialized: Boolean;
  end;

  TRadioBandIndex = (
    rbLW,
    rbMW,
    rbSW,
    rbFM
  );

  TRadioBandMemory = record
    FrequencyHz: Int64;
    Mode: string;
    Initialized: Boolean;
  end;

  TfrmMain = class(TForm)
    imgFacade: TImage;
    ion_off: TImage;
    hsPower: TImage;
    imgTube0: TImage;
    imgDigit0: TImage;
    imgGlass0: TImage;
    imgTube1: TImage;
    imgDigit1: TImage;
    imgGlass1: TImage;
    imgTube2: TImage;
    imgDigit2: TImage;
    imgGlass2: TImage;
    imgTube3: TImage;
    imgDigit3: TImage;
    imgGlass3: TImage;
    imgTube4: TImage;
    imgDigit4: TImage;
    imgGlass4: TImage;
    imgTube5: TImage;
    imgDigit5: TImage;
    imgGlass5: TImage;
    imgTube6: TImage;
    imgDigit6: TImage;
    imgGlass6: TImage;
    imgTube7: TImage;
    imgDigit7: TImage;
    imgGlass7: TImage;
    lblStatusDynamic: TLabel;
    tmrVisuals: TTimer;
    hsTuning: TImage;
    hsAM: TImage;
    hsFM: TImage;
    hsUSB: TImage;
    hsLSB: TImage;
    hsCW: TImage;
    hsCWR: TImage;
    hsScan: TImage;
    hsMem: TImage;
    hsBand: TImage;
    hsFilter: TImage;
    hsNB: TImage;
    hsNR: TImage;
    hsAGC: TImage;
    hsNotch: TImage;
    hsLock: TImage;
    hsHam160: TImage;
    hsHam80: TImage;
    hsHam40: TImage;
    hsHam20: TImage;
    hsHam17: TImage;
    hsHam15: TImage;
    hsHam12: TImage;
    hsHam10: TImage;
    hsHamCB: TImage;
    hsRadioSW: Timage;
    hsStep: TImage;
    pbMeterNeedle: TPaintBox;
    imgKnobMotion: TImage;
    imgTubeGlow0: TImage;
    imgTubeGlow1: TImage;
    imgTubeGlow2: TImage;
    imgKnobVolume: TImage;
    imgKnobSquelch: TImage;
    imgKnobBFO: TImage;
    imgKnobRFGain: TImage;
    imgKnobAFGain: TImage;
    imgKnobClarifier: TImage;
    imode: TImage;
    StandardColorMap1: TStandardColorMap;
    hsRadioMW: TImage;
    hsRadioFM: TImage;
    hsRadioLW: TImage;
    lblRDSPS: TLabel;
    lblRDSRT: TLabel;
    lblRDSPI: TLabel;
    lblRDSPTY: TLabel;
    lblRDSCT: TLabel;
    hsQuitter: TImage;
    txtBfo: TEdit;
    lblConnectionInfo: TLabel;
    HsCB: TImage;
    LblCanal: TLabel;

    procedure FormCreate(Sender: TObject);
    procedure ion_offClick(Sender: TObject);
    procedure hsPowerClick(Sender: TObject);
    procedure hsQuitterClick(Sender: TObject);
    procedure hsRadioFMClick(Sender: TObject);
    procedure HsCBClick(Sender: TObject);
    procedure NixieClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
    procedure hsTuningMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure hsTuningMouseMove(Sender: TObject; Shift: TShiftState;
      X, Y: Integer);
    procedure hsTuningMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure hsTuningMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
    procedure ModeHotspotClick(Sender: TObject);
    procedure hsScanClick(Sender: TObject);
    procedure hsLockClick(Sender: TObject);
    procedure hsStepClick(Sender: TObject);
    procedure tmrVisualsTimer(Sender: TObject);
  private

    FConnectionBusy: Boolean;
    FLastModeVisual: string;
    FLastNixieText: string;
    FLastRemoteMode: string;
    FRemoteModeConfirmCount: Integer;
    FLastRemoteFrequencyKHz: Int64;
    FRemoteFrequencyConfirmCount: Integer;
    FLocalControlUntil: UInt64;
    FSmoothedMeterLevel: Double;
    FApplyingRemoteStatus: Boolean;
    FRDSEnabledOnATS: Boolean;
    FRDSStation: string;
    FRDSText: string;
    FRDSPI: string;
    FRDSPTY: string;
    FRDSCT: string;
    FRTScrollSource: string;
    FRTScrollPos: Integer;
    FRTScrollDelay: Integer;
    FRTScrollTick: Integer;
    FRadioMemories: array[TRadioBandIndex] of TRadioBandMemory;
    FCurrentRadioBand: TRadioBandIndex;
    FHasCurrentRadioBand: Boolean;
    FInRadioBand: Boolean;
    FStatusPollCounter: Integer;
    FHamMemories: array[THamBandIndex] of THamBandMemory;
    FCurrentHamBand: THamBandIndex;
    FHasCurrentHamBand: Boolean;
    FATSConnection: TATSConnection;
    FConnectionTimer: TTimer;
    FTuningMoved: Boolean;
    FVolume: Integer;
    FSquelch: Integer;
    FBFO: Integer;
    FRFGain: Integer;
    FAFGain: Integer;
    FClarifier: Integer;
    FPowerOn: Boolean;
    FSerialMonitor: TfrmSerialMonitor;
    FFrequencyHz: Int64;
    FStepHz: Integer;
    FMode: string;
    FHamBand: string;
    FDragging: Boolean;
    FLastMouseY: Integer;
    FScanEnabled: Boolean;
    FScanPaused: Boolean;
    FScanStartTick: UInt64;
    FScanLastActivityTick: UInt64;
    FLocked: Boolean;
    FReceiverModel: string;
    FReceiverFirmware: string;
    FReceiverHardware: string;
    FReceiverChip: string;
    FNixieDigits: array[0..9] of TPicture;
    FNixieTube: TPicture;
    FNixieGlass: TPicture;
    FNixieDecimal: TShape;
    FKnobFrames: TSteamFrameSet;
    FTubeGlowFrames: TSteamFrameSet;
    FKnobFrame: Integer;
    FMeterLevel: Integer;
    FNeedle: TRotatingNeedle;
    FGlowPhase: Integer;
    FSmallKnobs: array[0..5] of TSteamKnob;
    FButtonManager: TSteamButtonManager;
    procedure LoadNixieAssets;
    procedure FreeNixieAssets;
    procedure ChangeFrequency(const Delta: Int64);
    function CBFrequencyForPosition(const APosition: Integer): Int64;
    function CBPositionForFrequency(const AFrequencyHz: Int64): Integer;
    function IsCBActive: Boolean;
    procedure UpdateDisplay;
    procedure UpdateNixieDisplay;
    procedure UpdateNixieDecimal(const ADigitsBeforePoint: Integer);
    procedure SetModeByIndex(const AIndex: Integer);
    function IsFMBand: Boolean;
    function IsBroadcastFMActive: Boolean;
    function IsSpectrumScanBandActive: Boolean;
    procedure EnforceFMBandMode;
    procedure LoadVisualAssets;
    procedure FreeVisualAssets;
    procedure UpdateModeLamps;
    procedure UpdateLockState;
    function IsFrontPanelLocked: Boolean;
    procedure UpdateModeButtons;
    procedure UpdateModeImage;
    procedure UpdateStepImage;
    procedure UpdateHamButtons;
    procedure InitializeRadioMemories;
    procedure SaveCurrentRadioBandMemory;
    procedure SelectRadioBand(const ABand: TRadioBandIndex);
    procedure ApplyRadioModeRules(const ABand: TRadioBandIndex);
    procedure UpdateRadioButtons;
    procedure InitializeHamMemories;
    procedure SaveCurrentHamBandMemory;
    procedure SelectHamBand(const ABand: THamBandIndex);
    procedure ApplyHamModeRules(const ABand: THamBandIndex);
    procedure GetActiveBandLimits(out AMinHz, AMaxHz: Int64);
    procedure SelectBandForDirectFrequency(const AFrequencyHz: Int64);
    procedure SyncBandFromStatus(const ABand: string);
    procedure UpdateKnobFrame(const ADirection: Integer);
    procedure SetMeterLevel(const ALevel: Integer);
    procedure UpdateSMeterFromRSSI(const ARSSI: Integer);
    procedure UpdatePowerDisplay;
    procedure UpdatePowerState;
    procedure ConnectionTimer(Sender: TObject);
    procedure ApplyATSStatus(const AStatus: TATSStatus);
    procedure PollATSStatus;
    procedure ParseRDSLine(const ALine: string);
    procedure UpdateRDSDisplay;
    procedure UpdateRDSRTScroll;
    procedure ClearRDSDisplay;
    procedure ATSConnectionLog(Sender: TObject; const AText: string);
    function SendATSCommand(const ACommand: AnsiString;
      ABlockRemote: Boolean = True): Boolean;
    function SendModeCommand(const AMode: string;
      ABlockRemote: Boolean = True): Boolean;
    function WaitForPong(ATimeoutMs: Cardinal): Boolean;
    function QueryWiFiIP(out AIP: string; ATimeoutMs: Cardinal): Boolean;
    procedure SmallKnobChanged(Sender: TObject);
    procedure UpdateKnobValues;
    procedure UpdateBFOText;
    procedure LoadButtonManager(const BasePath: string);
    procedure ButtonCommand(Sender: TObject; const ACommand: string;
      AState: Boolean);
    procedure UpdateConnectionInfo;
    procedure ShowAbout;
    procedure ShowFavorites;
    procedure RecallFavorite(const AFavorite: TFavoriteData);
    function CurrentFavoriteBand: string;
    procedure RequestReceiverId;
    procedure ParseReceiverId(const ALine: string);
    procedure SpectrumStopRequest(Sender: TObject);
    procedure SpectrumStartRequest(Sender: TObject;
      AStartMHz, AStepKHz: Double);
    procedure SpectrumPauseRequest(Sender: TObject);
    procedure SpectrumResumeRequest(Sender: TObject);
    procedure SpectrumScanEnded(Sender: TObject);
    function SpectrumTuneRequested(Sender: TObject;
      AFrequencyKHz: Int64): Boolean;
    procedure SpectrumFavoriteRequested(Sender: TObject);
    procedure AbortSpectrumScan(const AReason: string);
  end;

var
  frmMain: TfrmMain;

implementation

type
  TControlAccess = class(TControl);

{$R *.dfm}

const
  CMinFrequencyHz = Int64(100000);
  CMaxFrequencyHz = Int64(200000000);
  CHamMin: array[THamBandIndex] of Int64 =
    (1800000, 3500000, 7000000, 14000000, 18068000,
     21000000, 24890000, 28000000, 26515000);
  CHamMax: array[THamBandIndex] of Int64 =
    (2000000, 3800000, 7200000, 14350000, 18168000,
     21450000, 24990000, 29700000, 27855000);
  CRadioMin: array[TRadioBandIndex] of Int64 =
    (148500, 531000, 2300000, 87500000);
  CRadioMax: array[TRadioBandIndex] of Int64 =
    (283500, 1602000, 26100000, 108000000);
  CScanBeginTimeoutMs = UInt64(5000);
  CScanInactivityTimeoutMs = UInt64(30000);
  CScanTotalTimeoutMs = UInt64(120000);
  CCBChannelsPerBand = 40;
  CCBTotalPositions = 120;
  CCBBandOffsetHz = Int64(450000);
  CCBChannelFrequenciesHz: array[1..CCBChannelsPerBand] of Int64 = (
    26965000, 26975000, 26985000, 27005000, 27015000,
    27025000, 27035000, 27055000, 27065000, 27075000,
    27085000, 27105000, 27115000, 27125000, 27135000,
    27155000, 27165000, 27175000, 27185000, 27205000,
    27215000, 27225000, 27255000, 27235000, 27245000,
    27265000, 27275000, 27285000, 27295000, 27305000,
    27315000, 27325000, 27335000, 27345000, 27355000,
    27365000, 27375000, 27385000, 27395000, 27405000
  );

procedure TfrmMain.RequestReceiverId;
begin
  if (FATSConnection <> nil) and FATSConnection.IsAlive then
    SendATSCommand(AnsiString('ID?'#10), False);
end;

procedure TfrmMain.ParseReceiverId(const ALine: string);
var
  Parts: TArray<string>;
  P, K, V: string;
  I, EqPos: Integer;
begin
  if not ALine.StartsWith('ID,', True) then Exit;

  Parts := ALine.Split([',']);
  if Length(Parts) < 2 then Exit;

  FReceiverModel := Parts[1];

  for I := 2 to High(Parts) do
  begin
    P := Parts[I];
    EqPos := P.IndexOf('=');
    if EqPos <= 0 then Continue;

    K := P.Substring(0, EqPos).Trim.ToUpper;
    V := P.Substring(EqPos + 1).Trim;

    if K = 'FW' then FReceiverFirmware := V
    else if K = 'HW' then FReceiverHardware := V
    else if K = 'CHIP' then FReceiverChip := V;
  end;
end;

procedure TfrmMain.ShowAbout;
var
  ConnType, ConnDetail, ReceiverText, FirmwareText: string;
begin
  ConnType := 'NON CONNECTE';
  ConnDetail := '-';

  if (FATSConnection <> nil) and FATSConnection.IsAlive then
  begin
    case FATSConnection.Transport of
      attSerial:
        begin
          ConnType := 'USB';
          ConnDetail := Format('%s @ %d bauds',
            [FATSConnection.PortName, FATSConnection.BaudRate]);
        end;
      attWiFi:
        begin
          ConnType := 'WIFI';
          ConnDetail := Format('%s:%d',
            [FATSConnection.Host, FATSConnection.TcpPort]);
        end;
    end;
  end;

  if FReceiverModel <> '' then ReceiverText := FReceiverModel
  else ReceiverText := 'ATS-25 / SI4735';

  FirmwareText := '';
  if FReceiverFirmware <> '' then
    FirmwareText := 'FW ' + FReceiverFirmware;
  if FReceiverHardware <> '' then
  begin
    if FirmwareText <> '' then FirmwareText := FirmwareText + ' - ';
    FirmwareText := FirmwareText + FReceiverHardware;
  end;
  if FReceiverChip <> '' then
  begin
    if FirmwareText <> '' then FirmwareText := FirmwareText + ' / ';
    FirmwareText := FirmwareText + FReceiverChip;
  end;
  if FirmwareText = '' then
    FirmwareText := 'Identification non recue';

  TAboutForm.Execute(
    'ATS LAB',
    '1.1.1',
    ReceiverText,
    ConnType,
    ConnDetail,
    FirmwareText,
    'O.H.M',
    'https://github.com/jla1810/AtsLabSteam'
  );
end;

function TfrmMain.CurrentFavoriteBand: string;
begin
  Result := '';
  if FHasCurrentHamBand then
    Result := FHamBand
  else if FInRadioBand and FHasCurrentRadioBand then
  begin
    case FCurrentRadioBand of
      rbLW: Result := 'LW';
      rbMW: Result := 'MW';
      rbSW: Result := 'SW';
      rbFM: Result := 'FM';
    end;
  end;
end;

procedure TfrmMain.ShowFavorites;
var
  Current, Selected: TFavoriteData;
  IniFileName: string;
begin
  if not FPowerOn then
    Exit;

  Current.Name := '';
  Current.Band := CurrentFavoriteBand;
  Current.FrequencyHz := FFrequencyHz;
  Current.Mode := FMode;
  Current.Volume := FVolume;
  Current.Squelch := FSquelch;
  Current.BFO := FBFO;
  IniFileName := ChangeFileExt(Application.ExeName, '.ini');

  if TfrmFavorites.Execute(IniFileName, Current, Selected) then
    RecallFavorite(Selected);
end;

procedure TfrmMain.RecallFavorite(const AFavorite: TFavoriteData);
var
  MinHz, MaxHz: Int64;
  Band, Mode: string;
begin
  if not FPowerOn then
    Exit;
  if (AFavorite.FrequencyHz < CMinFrequencyHz) or
     (AFavorite.FrequencyHz > CMaxFrequencyHz) or
     ((AFavorite.FrequencyHz > 30000000) and
      (AFavorite.FrequencyHz < 87500000)) then
  begin
    MessageDlg('La fréquence enregistrée dans ce favori n''est pas prise en charge.',
      mtError, [mbOK], 0);
    Exit;
  end;

  Band := UpperCase(Trim(AFavorite.Band));
  if (Band <> '') and not MatchText(Band,
    ['160', '80', '40', '20', '17', '15', '12', '10', 'CB',
     'LW', 'MW', 'SW', 'FM']) then
  begin
    MessageDlg('La bande enregistrée dans ce favori n''est pas valide.',
      mtError, [mbOK], 0);
    Exit;
  end;

  if FScanEnabled then
    AbortSpectrumScan('Scan arrêté par le rappel d''un favori.');

  if Band <> '' then
    SyncBandFromStatus(Band)
  else
    SelectBandForDirectFrequency(AFavorite.FrequencyHz);

  GetActiveBandLimits(MinHz, MaxHz);
  if (AFavorite.FrequencyHz < MinHz) or
     (AFavorite.FrequencyHz > MaxHz) then
  begin
    MessageDlg(Format(
      'Le favori "%s" contient une fréquence hors de sa bande %s.',
      [AFavorite.Name, AFavorite.Band]), mtError, [mbOK], 0);
    Exit;
  end;

  FFrequencyHz := AFavorite.FrequencyHz;
  Mode := UpperCase(Trim(AFavorite.Mode));
  if Mode = 'CWR' then
    Mode := 'CW-R';
  if not MatchText(Mode, ['AM', 'FM', 'USB', 'LSB', 'CW', 'CW-R']) then
    Mode := 'AM';
  FMode := Mode;
  if FHasCurrentHamBand then
    ApplyHamModeRules(FCurrentHamBand)
  else if FInRadioBand and FHasCurrentRadioBand then
    ApplyRadioModeRules(FCurrentRadioBand);

  FVolume := EnsureRange(AFavorite.Volume, 0, 100);
  FSquelch := EnsureRange(AFavorite.Squelch, 0, 100);
  FBFO := EnsureRange(AFavorite.BFO, -3000, 3000);
  FLocalControlUntil := GetTickCount64 + 1500;

  UpdateDisplay;
  UpdateModeImage;
  UpdateModeButtons;
  UpdateModeLamps;
  UpdateBFOText;
  UpdateKnobValues;

  if (FATSConnection <> nil) and FATSConnection.IsAlive then
  begin
    SendATSCommand(TATSProtocol.SetFrequencyKHz(FFrequencyHz div 1000));
    SendModeCommand(FMode);
    SendATSCommand(TATSProtocol.SetVolume(FVolume));
    SendATSCommand(TATSProtocol.SetSquelch(FSquelch));
    if MatchText(FMode, ['USB', 'LSB', 'CW', 'CW-R']) then
      SendATSCommand(TATSProtocol.SetBFO(FBFO));
  end;

  SaveCurrentHamBandMemory;
  if FInRadioBand then
    SaveCurrentRadioBandMemory;
end;


procedure TfrmMain.FormCreate(Sender: TObject);
begin
  FConnectionBusy := False;
  FReceiverModel := '';
  FReceiverFirmware := '';
  FReceiverHardware := '';
  FReceiverChip := '';

  FLastModeVisual := '';
  FLastNixieText := '';
  FLastRemoteMode := '';
  FRemoteModeConfirmCount := 0;
  FLastRemoteFrequencyKHz := -1;
  FRemoteFrequencyConfirmCount := 0;
  FLocalControlUntil := 0;
  FSmoothedMeterLevel := 0;
  FApplyingRemoteStatus := False;
  FRDSEnabledOnATS := False;
  FRDSStation := '';
  FRDSText := '';
  FRDSPI := '';
  FRDSPTY := '';
  FRDSCT := '';
  FRTScrollSource := '';
  FRTScrollPos := 1;
  FRTScrollDelay := 0;
  FRTScrollTick := 0;
  FATSConnection := TATSConnection.Create;
  FConnectionTimer := TTimer.Create(Self);
  FConnectionTimer.Interval := 1000;
  FConnectionTimer.Enabled := False;
  FConnectionTimer.OnTimer := ConnectionTimer;
FPowerOn := False;
  FVolume := 50;
  FSquelch := 25;
  FBFO := 0;
  FRFGain := 80;
  FAFGain := 50;
  FClarifier := 0;
  FHamBand := '20';
  FFrequencyHz := 14205000;
  FStepHz := 1000;
  FMode := 'USB';
  FDragging := False;
  FScanEnabled := False;
  FScanPaused := False;
  FScanStartTick := 0;
  FScanLastActivityTick := 0;
  frmSpectrum := TfrmSpectrum.Create(Self);
  frmSpectrum.OnStopRequest := SpectrumStopRequest;
  frmSpectrum.OnScanEnd := SpectrumScanEnded;
  frmSpectrum.OnTuneRequest := SpectrumTuneRequested;
  frmSpectrum.OnStartRequest := SpectrumStartRequest;
  frmSpectrum.OnPauseRequest := SpectrumPauseRequest;
  frmSpectrum.OnResumeRequest := SpectrumResumeRequest;
  frmSpectrum.OnFavoriteRequest := SpectrumFavoriteRequested;
  FLocked := False;
  InitializeHamMemories;
  InitializeRadioMemories;

  Randomize;
  FKnobFrame := 24;
  FMeterLevel := 18;
  FGlowPhase := 0;
  FStatusPollCounter := 0;

  TGraphicsCache.Initialize(
    TPath.Combine(ExtractFilePath(Application.ExeName), 'Data')
  );

  LoadNixieAssets;
  LoadVisualAssets;
  UpdateModeImage;
  UpdateStepImage;
  UpdateHamButtons;
  UpdateDisplay;
  UpdateModeLamps;
  UpdateModeButtons;
  UpdateRDSDisplay;
  FSerialMonitor := TfrmSerialMonitor.Create(Self);
  FSerialMonitor.Left := Left + Width;
  FSerialMonitor.Top := Top;
  FSerialMonitor.Show;
  FATSConnection.OnLog := ATSConnectionLog;
  UpdatePowerDisplay;
  UpdateKnobValues;
  UpdateBFOText;
  UpdatePowerState;
  TControlAccess(hsTuning).OnMouseWheel := hsTuningMouseWheel;
  hsTuning.BringToFront;
  UpdateConnectionInfo;
  UpdateLockState;
end;

procedure TfrmMain.hsQuitterClick(Sender: TObject);
begin
  { Arrêt propre avant fermeture }
  if FConnectionTimer <> nil then
    FConnectionTimer.Enabled := False;

  if FScanEnabled then
    AbortSpectrumScan('Scan arrêté avant la fermeture.');

  if FATSConnection <> nil then
    FATSConnection.Disconnect;
  lblConnectionInfo.Caption := 'Connexion : NON CONNECTE';

  FRDSEnabledOnATS := False;
  FPowerOn := False;
  UpdatePowerState;

  Close;
end;

procedure TfrmMain.FormDestroy(Sender: TObject);
begin
  if frmSpectrum <> nil then
  begin
    frmSpectrum.OnStopRequest := nil;
    frmSpectrum.OnScanEnd := nil;
    frmSpectrum.OnTuneRequest := nil;
    frmSpectrum.OnStartRequest := nil;
    frmSpectrum.OnPauseRequest := nil;
    frmSpectrum.OnResumeRequest := nil;
    frmSpectrum.OnFavoriteRequest := nil;
  end;
  FreeAndNil(frmSpectrum);
if FConnectionTimer <> nil then
    FConnectionTimer.Enabled := False;
  FreeAndNil(FATSConnection);
  FreeAndNil(FSerialMonitor);
  FreeVisualAssets;
  FreeNixieAssets;
  TGraphicsCache.Finalize;
end;

procedure TfrmMain.LoadNixieAssets;
var
  I: Integer;
  BasePath: string;
  FileName: string;
  Tubes: array[0..7] of TImage;
  Glasses: array[0..7] of TImage;
begin
  BasePath := TPath.Combine(
    TPath.Combine(ExtractFilePath(Application.ExeName), 'Data'),
    'Nixie'
  );

  FNixieTube := TPicture.Create;
  FNixieGlass := TPicture.Create;

  FileName := TPath.Combine(BasePath, 'Tube.png');
  if not FileExists(FileName) then
    raise Exception.CreateFmt('Tube Nixie introuvable : %s', [FileName]);
  TGraphicsCache.AssignTo(FNixieTube, FileName);

  FileName := TPath.Combine(BasePath, 'Glass.png');
  if not FileExists(FileName) then
    raise Exception.CreateFmt('Reflet Nixie introuvable : %s', [FileName]);
  TGraphicsCache.AssignTo(FNixieGlass, FileName);

  for I := 0 to 9 do
  begin
    FNixieDigits[I] := TPicture.Create;
    FileName := TPath.Combine(
      TPath.Combine(BasePath, 'Digits'),
      Format('%d.png', [I])
    );

    if not FileExists(FileName) then
      raise Exception.CreateFmt('Chiffre Nixie introuvable : %s', [FileName]);

    TGraphicsCache.AssignTo(FNixieDigits[I], FileName);
  end;

  Tubes[0] := imgTube0; Tubes[1] := imgTube1;
  Tubes[2] := imgTube2; Tubes[3] := imgTube3;
  Tubes[4] := imgTube4; Tubes[5] := imgTube5;
  Tubes[6] := imgTube6; Tubes[7] := imgTube7;

  Glasses[0] := imgGlass0; Glasses[1] := imgGlass1;
  Glasses[2] := imgGlass2; Glasses[3] := imgGlass3;
  Glasses[4] := imgGlass4; Glasses[5] := imgGlass5;
  Glasses[6] := imgGlass6; Glasses[7] := imgGlass7;

  for I := 0 to 7 do
  begin
    Tubes[I].Picture.Assign(FNixieTube);
    Glasses[I].Picture.Assign(FNixieGlass);
  end;
  { Point décimal indépendant des 8 tubes.
    Il se déplace selon la gamme de fréquence. }
  FNixieDecimal := TShape.Create(Self);
  FNixieDecimal.Parent := imgDigit0.Parent;
  FNixieDecimal.Shape := stCircle;
  FNixieDecimal.Width := 8;
  FNixieDecimal.Height := 8;
  FNixieDecimal.Brush.Color := RGB(255, 120, 0);
  FNixieDecimal.Pen.Style := psClear;
  FNixieDecimal.Top := imgDigit0.Top + imgDigit0.Height - 20;
  FNixieDecimal.Visible := FPowerOn;
  FNixieDecimal.BringToFront;
end;

procedure TfrmMain.FreeNixieAssets;
var
  I: Integer;
begin
  FreeAndNil(FNixieDecimal);
  for I := 0 to 9 do
    FreeAndNil(FNixieDigits[I]);

  FreeAndNil(FNixieTube);
  FreeAndNil(FNixieGlass);
end;

procedure TfrmMain.LoadVisualAssets;
var
  BasePath: string;
  I: Integer;
begin
  BasePath := TPath.Combine(ExtractFilePath(Application.ExeName), 'Data');

  FSmallKnobs[0] := TSteamKnob.Create(
    imgKnobVolume,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    0, 100, 50, 1
  );
  FSmallKnobs[1] := TSteamKnob.Create(
    imgKnobSquelch,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    0, 100, 25, 1
  );
  FSmallKnobs[2] := TSteamKnob.Create(
    imgKnobBFO,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    -3000, 3000, 0, 50
  );
  FSmallKnobs[3] := TSteamKnob.Create(
    imgKnobRFGain,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    0, 100, 80, 1
  );
  FSmallKnobs[4] := TSteamKnob.Create(
    imgKnobAFGain,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    0, 100, 50, 1
  );
  FSmallKnobs[5] := TSteamKnob.Create(
    imgKnobClarifier,
    TPath.Combine(TPath.Combine(BasePath, 'SmallKnobs'), 'Frames'),
    -1500, 1500, 0, 25
  );

  for I := 0 to 5 do
    FSmallKnobs[I].OnChange := SmallKnobChanged;

  LoadButtonManager(BasePath);

  FKnobFrames := TSteamFrameSet.Create(
    TPath.Combine(TPath.Combine(BasePath, 'Knob'), 'Frames'),
    'knob_%.2d.png', 48
  );


  FTubeGlowFrames := TSteamFrameSet.Create(
    TPath.Combine(BasePath, 'VacuumTubes'),
    'glow_%d.png', 3
  );

  FKnobFrames.AssignTo(imgKnobMotion, FKnobFrame);
  FNeedle := TRotatingNeedle.Create(
    pbMeterNeedle,
    TPath.Combine(TPath.Combine(BasePath, 'Meter'), 'Needle.png'),
    269, 177
  );
  SetMeterLevel(FMeterLevel);
  FTubeGlowFrames.AssignTo(imgTubeGlow0, 1);
  FTubeGlowFrames.AssignTo(imgTubeGlow1, 1);
  FTubeGlowFrames.AssignTo(imgTubeGlow2, 1);

  imgTubeGlow0.Visible := True;
  imgTubeGlow1.Visible := True;
  imgTubeGlow2.Visible := True;
  imgTubeGlow0.BringToFront;
  imgTubeGlow1.BringToFront;
  imgTubeGlow2.BringToFront;
end;

procedure TfrmMain.FreeVisualAssets;
var
  I: Integer;
begin
  FreeAndNil(FButtonManager);

  for I := 0 to 5 do
    FreeAndNil(FSmallKnobs[I]);

  FreeAndNil(FNeedle);
  FreeAndNil(FKnobFrames);
  FreeAndNil(FTubeGlowFrames);
end;

procedure TfrmMain.LoadButtonManager(const BasePath: string);
var
  Folder, OffFile, OnFile, PressedFile: string;
  HamOffFile, HamOnFile: string;
begin
  Folder := TPath.Combine(BasePath, 'Buttons');
  OffFile := TPath.Combine(Folder, 'button_off.png');
  OnFile := TPath.Combine(Folder, 'button_on.png');
  PressedFile := TPath.Combine(Folder, 'button_pressed.png');
  HamOffFile := TPath.Combine(Folder, 'small_button_off.png');
  HamOnFile := TPath.Combine(Folder, 'small_button_on.png');

  FButtonManager := TSteamButtonManager.Create;
  FButtonManager.OnCommand := ButtonCommand;

  FButtonManager.AddButton(hsAM, 1, sbkRadio, 'AM',
    OffFile, OnFile, PressedFile, SameText(FMode, 'AM'));
  FButtonManager.AddButton(hsFM, 1, sbkRadio, 'FM',
    OffFile, OnFile, PressedFile, SameText(FMode, 'FM'));
  FButtonManager.AddButton(hsUSB, 1, sbkRadio, 'USB',
    OffFile, OnFile, PressedFile, SameText(FMode, 'USB'));
  FButtonManager.AddButton(hsLSB, 1, sbkRadio, 'LSB',
    OffFile, OnFile, PressedFile, SameText(FMode, 'LSB'));
  FButtonManager.AddButton(hsCW, 1, sbkRadio, 'CW',
    OffFile, OnFile, PressedFile, SameText(FMode, 'CW'));
  FButtonManager.AddButton(hsCWR, 1, sbkRadio, 'CW-R',
    OffFile, OnFile, PressedFile, SameText(FMode, 'CW-R'));
  FButtonManager.AddButton(hsScan, 2, sbkToggle, 'SCAN',
    OffFile, OnFile, PressedFile, FScanEnabled);
  FButtonManager.AddButton(hsMem, 3, sbkToggle, 'MEM',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsBand, 3, sbkToggle, 'BAND',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsFilter, 3, sbkToggle, 'FILTER',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsNB, 3, sbkToggle, 'NB',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsNR, 3, sbkToggle, 'NR',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsAGC, 3, sbkToggle, 'AGC',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsNotch, 3, sbkToggle, 'NOTCH',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsLock, 3, sbkToggle, 'LOCK',
    OffFile, OnFile, PressedFile, False);
  FButtonManager.AddButton(hsHam160, 10, sbkRadio, 'HAM160',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam80, 10, sbkRadio, 'HAM80',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam40, 10, sbkRadio, 'HAM40',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam20, 10, sbkRadio, 'HAM20',
    HamOffFile, HamOnFile, HamOffFile, True);
  FButtonManager.AddButton(hsHam17, 10, sbkRadio, 'HAM17',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam15, 10, sbkRadio, 'HAM15',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam12, 10, sbkRadio, 'HAM12',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHam10, 10, sbkRadio, 'HAM10',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsHamCB, 10, sbkRadio, 'CB',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsRadioLW, 11, sbkRadio, 'RADIO_LW',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsRadioMW, 11, sbkRadio, 'RADIO_MW',
    HamOffFile, HamOnFile, HamOffFile, False);
  FButtonManager.AddButton(hsRadioSW, 11, sbkRadio, 'RADIO_SW',
    HamOffFile, HamOnFile, HamOffFile, False);
end;

procedure TfrmMain.ButtonCommand(Sender: TObject;
  const ACommand: string; AState: Boolean);
begin
    { LOCK doit toujours rester accessible, même quand la façade est verrouillée. }
  if SameText(ACommand, 'LOCK') then
  begin
    FLocked := AState;
    UpdateLockState;
    Exit;
  end;

  { Toutes les autres commandes de façade sont bloquées en mode LOCK. }
  if IsFrontPanelLocked then
    Exit;

  if SameText(ACommand, 'RADIO_LW') then
  begin
    SelectRadioBand(rbLW);
    Exit;
  end
  else if SameText(ACommand, 'RADIO_MW') then
  begin
    SelectRadioBand(rbMW);
    Exit;
  end
  else if SameText(ACommand, 'RADIO_SW') then
  begin
    SelectRadioBand(rbSW);
    Exit;
  end;

  if SameText(ACommand, 'HAM160') then
  begin
    SelectHamBand(hb160m);
    Exit;
  end
  else if SameText(ACommand, 'HAM80') then
  begin
    SelectHamBand(hb80m);
    Exit;
  end
  else if SameText(ACommand, 'HAM40') then
  begin
    SelectHamBand(hb40m);
    Exit;
  end
  else if SameText(ACommand, 'HAM20') then
  begin
    SelectHamBand(hb20m);
    Exit;
  end
  else if SameText(ACommand, 'HAM17') then
  begin
    SelectHamBand(hb17m);
    Exit;
  end
  else if SameText(ACommand, 'HAM15') then
  begin
    SelectHamBand(hb15m);
    Exit;
  end
  else if SameText(ACommand, 'HAM12') then
  begin
    SelectHamBand(hb12m);
    Exit;
  end
  else if SameText(ACommand, 'HAM10') then
  begin
    SelectHamBand(hb10m);
    Exit;
  end
  else if SameText(ACommand, 'CB') then
  begin
    SelectHamBand(hbCB);
    Exit;
  end;

  if SameText(ACommand, 'SCAN') then
  begin
    { Le gestionnaire de boutons remplace le OnClick DFM.
      Rediriger donc vers le vrai traitement Scan/Spectrum. }
    hsScanClick(Sender);
    Exit;
  end;

  if SameText(ACommand, 'MEM') then
  begin
    ShowFavorites;
    if FButtonManager <> nil then
      FButtonManager.SetState('MEM', False);
    Exit;
  end;

  if SameText(ACommand, 'AM') or
     SameText(ACommand, 'FM') or
     SameText(ACommand, 'USB') or
     SameText(ACommand, 'LSB') or
     SameText(ACommand, 'CW') or
     SameText(ACommand, 'CW-R') then
  begin
    if (Sender is TControl) and not TControl(Sender).Enabled then
      Exit;

    FMode := ACommand;
    if FInRadioBand then
      SaveCurrentRadioBandMemory;
    SaveCurrentHamBandMemory;
    UpdateModeImage;
    UpdateModeLamps;
    UpdateDisplay;

    if (not FApplyingRemoteStatus) and FPowerOn and
       (FATSConnection <> nil) and
       FATSConnection.IsAlive then
      SendModeCommand(FMode);

    Exit;
  end;
end;


procedure TfrmMain.UpdateStepImage;
var
  StepFile: string;
begin
  case FStepHz of
    1000: StepFile := '1k.png';
    10000: StepFile := '10k.png';
    25000: StepFile := '25k.png';
    100000: StepFile := '100k.png';
  else
    begin
      FStepHz := 1000;
      StepFile := '1k.png';
    end;
  end;

  StepFile := TPath.Combine(
    TPath.Combine(ExtractFilePath(Application.ExeName), 'Data\Mode'),
    StepFile
  );

  if FileExists(StepFile) then
    TGraphicsCache.AssignTo(hsStep.Picture, StepFile)
  else
    hsStep.Picture.Assign(nil);
end;

procedure TfrmMain.UpdateModeImage;
var
  ModeName: string;
  ModeFile: string;
begin
  if SameText(FLastModeVisual, FMode) then
    Exit;

  ModeName := LowerCase(StringReplace(FMode, '-', '', [rfReplaceAll]));
  ModeFile := TPath.Combine(
    TPath.Combine(ExtractFilePath(Application.ExeName), 'Data\Mode'),
    ModeName + '.png'
  );

  if SameText(FMode, 'CW-R') and not FileExists(ModeFile) then
    ModeFile := TPath.Combine(
      TPath.Combine(ExtractFilePath(Application.ExeName), 'Data\Mode'),
      'cw.png'
    );

  if FileExists(ModeFile) then
    TGraphicsCache.AssignTo(imode.Picture, ModeFile)
  else
    imode.Picture.Assign(nil);
  FLastModeVisual := FMode;
end;

procedure TfrmMain.InitializeHamMemories;
var
  Band: THamBandIndex;
begin
  for Band := Low(THamBandIndex) to High(THamBandIndex) do
  begin
    FHamMemories[Band].FrequencyHz := 0;
    FHamMemories[Band].Mode := '';
    FHamMemories[Band].Initialized := False;
  end;

  FHasCurrentHamBand := False;
end;

procedure TfrmMain.SaveCurrentHamBandMemory;
begin
  if not FHasCurrentHamBand then
    Exit;

  if (FFrequencyHz < CHamMin[FCurrentHamBand]) or
     (FFrequencyHz > CHamMax[FCurrentHamBand]) then
    Exit;

  FHamMemories[FCurrentHamBand].FrequencyHz := FFrequencyHz;
  FHamMemories[FCurrentHamBand].Mode := FMode;
  FHamMemories[FCurrentHamBand].Initialized := True;
end;

procedure TfrmMain.ApplyHamModeRules(const ABand: THamBandIndex);
var
  AllowLSB: Boolean;
  AllowUSB: Boolean;
  AllowFM: Boolean;
  AllowCW: Boolean;
begin
  AllowLSB := ABand in [hb160m, hb80m, hb40m, hbCB];
  AllowUSB := ABand in [hb20m, hb17m, hb15m, hb12m, hb10m, hbCB];
  AllowFM := ABand in [hb10m, hbCB];
  AllowCW := ABand <> hbCB;

  hsAM.Enabled := True;
  hsLSB.Enabled := AllowLSB;
  hsUSB.Enabled := AllowUSB;
  hsCW.Enabled := AllowCW;
  { Le firmware fourni ne possède pas de mode CW-R distinct. }
  hsCWR.Enabled := False;
  hsFM.Enabled := AllowFM;

  if hsAM.Enabled then hsAM.Cursor := crHandPoint else hsAM.Cursor := crDefault;
  if hsLSB.Enabled then hsLSB.Cursor := crHandPoint else hsLSB.Cursor := crDefault;
  if hsUSB.Enabled then hsUSB.Cursor := crHandPoint else hsUSB.Cursor := crDefault;
  if hsCW.Enabled then hsCW.Cursor := crHandPoint else hsCW.Cursor := crDefault;
  if hsCWR.Enabled then hsCWR.Cursor := crHandPoint else hsCWR.Cursor := crDefault;
  if hsFM.Enabled then hsFM.Cursor := crHandPoint else hsFM.Cursor := crDefault;

  if (SameText(FMode, 'LSB') and not AllowLSB) or
     (SameText(FMode, 'USB') and not AllowUSB) or
     (SameText(FMode, 'FM') and not AllowFM) or
     (SameText(FMode, 'CW') and not AllowCW) or
     ((SameText(FMode, 'CW-R') or SameText(FMode, 'CWR')) and not AllowCW) then
  begin
    if AllowLSB then
      FMode := 'LSB'
    else
      FMode := 'USB';
  end;

  UpdateModeImage;
  UpdateModeLamps;
end;

procedure TfrmMain.SelectHamBand(const ABand: THamBandIndex);
const
  { Frequences de depart placees a l'interieur des bandes HAM.
    On evite volontairement les frontieres communes avec les bandes broadcast. }
  CMinFrequencyHz: array[THamBandIndex] of Int64 = (
    1850000,   { 160 m }
    3630000,   { 80 m  }
    7074000,   { 40 m  }
    14074000,  { 20 m  }
    18100000,  { 17 m  }
    21074000,  { 15 m  }
    24940000,  { 12 m  }
    28500000,  { 10 m  }
    27185000   { CB    }
  );
  CDefaultMode: array[THamBandIndex] of string = (
    'LSB', 'LSB', 'LSB', 'USB', 'USB',
    'USB', 'USB', 'USB', 'AM'
  );
  CBandName: array[THamBandIndex] of string = (
    '160', '80', '40', '20', '17',
    '15', '12', '10', 'CB'
  );
begin
  SaveCurrentRadioBandMemory;
  SaveCurrentHamBandMemory;
  FInRadioBand := False;
  FHasCurrentRadioBand := False;

  FCurrentHamBand := ABand;
  FHasCurrentHamBand := True;
  FHamBand := CBandName[ABand];

  if FHamMemories[ABand].Initialized then
  begin
    FFrequencyHz := FHamMemories[ABand].FrequencyHz;
    FMode := FHamMemories[ABand].Mode;
  end
  else
  begin
    FFrequencyHz := CMinFrequencyHz[ABand];
    FMode := CDefaultMode[ABand];
    FHamMemories[ABand].FrequencyHz := FFrequencyHz;
    FHamMemories[ABand].Mode := FMode;
    FHamMemories[ABand].Initialized := True;
  end;

  ApplyHamModeRules(ABand);
  UpdateHamButtons;
  UpdateDisplay;

  if FPowerOn and (FATSConnection <> nil) and
     FATSConnection.IsAlive then
  begin
    SendATSCommand(
      TATSProtocol.SetFrequencyKHz(FFrequencyHz div 1000)
    );

    SendModeCommand(FMode);
  end;
end;

procedure TfrmMain.InitializeRadioMemories;
var
  Band: TRadioBandIndex;
begin
  for Band := Low(TRadioBandIndex) to High(TRadioBandIndex) do
  begin
    FRadioMemories[Band].FrequencyHz := 0;
    FRadioMemories[Band].Mode := '';
    FRadioMemories[Band].Initialized := False;
  end;

  FHasCurrentRadioBand := False;
  FInRadioBand := False;
end;

procedure TfrmMain.SaveCurrentRadioBandMemory;
begin
  if not FHasCurrentRadioBand then
    Exit;

  if (FFrequencyHz < CRadioMin[FCurrentRadioBand]) or
     (FFrequencyHz > CRadioMax[FCurrentRadioBand]) then
    Exit;

  FRadioMemories[FCurrentRadioBand].FrequencyHz := FFrequencyHz;
  FRadioMemories[FCurrentRadioBand].Mode := FMode;
  FRadioMemories[FCurrentRadioBand].Initialized := True;
end;

procedure TfrmMain.ApplyRadioModeRules(const ABand: TRadioBandIndex);
var
  OnlyAM: Boolean;
begin
  if ABand = rbFM then
  begin
    hsAM.Enabled := False;
    hsFM.Enabled := True;
    hsLSB.Enabled := False;
    hsUSB.Enabled := False;
    hsCW.Enabled := False;
    hsCWR.Enabled := False;

    hsAM.Cursor := crDefault;
    hsFM.Cursor := crHandPoint;
    hsLSB.Cursor := crDefault;
    hsUSB.Cursor := crDefault;
    hsCW.Cursor := crDefault;
    hsCWR.Cursor := crDefault;

    FMode := 'FM';
    UpdateModeImage;
    UpdateModeLamps;
    Exit;
  end;

  OnlyAM := ABand in [rbLW, rbMW];

  hsAM.Enabled := True;
  hsFM.Enabled := False;
  hsLSB.Enabled := not OnlyAM;
  hsUSB.Enabled := not OnlyAM;
  hsCW.Enabled := not OnlyAM;
  hsCWR.Enabled := False;

  if hsAM.Enabled then hsAM.Cursor := crHandPoint else hsAM.Cursor := crDefault;
  if hsFM.Enabled then hsFM.Cursor := crHandPoint else hsFM.Cursor := crDefault;
  if hsLSB.Enabled then hsLSB.Cursor := crHandPoint else hsLSB.Cursor := crDefault;
  if hsUSB.Enabled then hsUSB.Cursor := crHandPoint else hsUSB.Cursor := crDefault;
  if hsCW.Enabled then hsCW.Cursor := crHandPoint else hsCW.Cursor := crDefault;
  if hsCWR.Enabled then hsCWR.Cursor := crHandPoint else hsCWR.Cursor := crDefault;

  if OnlyAM then
    FMode := 'AM'
  else if SameText(FMode, 'FM') then
    FMode := 'AM';

  UpdateModeImage;
  UpdateModeLamps;
end;

procedure TfrmMain.UpdateRadioButtons;
const
  CHamCommands: array[0..8] of string = (
    'HAM160', 'HAM80', 'HAM40', 'HAM20', 'HAM17',
    'HAM15', 'HAM12', 'HAM10', 'CB'
  );
var
  I: Integer;
begin
  if (FButtonManager = nil) or not FInRadioBand then
    Exit;

  { Eteindre explicitement le groupe HAM. }
  for I := Low(CHamCommands) to High(CHamCommands) do
    FButtonManager.SetState(CHamCommands[I], False);

  FButtonManager.SetState('RADIO_LW', FCurrentRadioBand = rbLW);
  FButtonManager.SetState('RADIO_MW', FCurrentRadioBand = rbMW);
  FButtonManager.SetState('RADIO_SW', FCurrentRadioBand = rbSW);
  { FM n'a pas de bouton géré par TSteamButtonManager ; son état est rendu
    par hsRadioFM et les modes. Les groupes HAM/LW/MW/SW sont néanmoins éteints. }
end;

procedure TfrmMain.SelectRadioBand(const ABand: TRadioBandIndex);
const
  CMinFrequencyHz: array[TRadioBandIndex] of Int64 = (
    148500,
    531000,
    2300000,
    88000000
  );
  CDefaultMode: array[TRadioBandIndex] of string = (
    'AM',
    'AM',
    'AM',
    'FM'
  );
begin
  SaveCurrentHamBandMemory;
  SaveCurrentRadioBandMemory;

  FHasCurrentHamBand := False;
  FInRadioBand := True;
  FCurrentRadioBand := ABand;
  FHasCurrentRadioBand := True;

  if FRadioMemories[ABand].Initialized then
  begin
    FFrequencyHz := FRadioMemories[ABand].FrequencyHz;
    FMode := FRadioMemories[ABand].Mode;
  end
  else
  begin
    FFrequencyHz := CMinFrequencyHz[ABand];
    FMode := CDefaultMode[ABand];
    FRadioMemories[ABand].FrequencyHz := FFrequencyHz;
    FRadioMemories[ABand].Mode := FMode;
    FRadioMemories[ABand].Initialized := True;
  end;

  ApplyRadioModeRules(ABand);
  UpdateRadioButtons;
  UpdateDisplay;

  if FPowerOn and (FATSConnection <> nil) and
     FATSConnection.IsAlive then
  begin
    SendATSCommand(
      TATSProtocol.SetFrequencyKHz(FFrequencyHz div 1000)
    );

    SendModeCommand(FMode);
  end;
end;

procedure TfrmMain.UpdateHamButtons;
const
  CHamCommands: array[0..8] of string = (
    'HAM160', 'HAM80', 'HAM40', 'HAM20', 'HAM17',
    'HAM15', 'HAM12', 'HAM10', 'CB'
  );
var
  I: Integer;
begin
  if FButtonManager = nil then
    Exit;

  { Eteindre explicitement le groupe radio. }
  FButtonManager.SetState('RADIO_LW', False);
  FButtonManager.SetState('RADIO_MW', False);
  FButtonManager.SetState('RADIO_SW', False);

  if FHamBand = '' then
    FHamBand := '20';

  for I := Low(CHamCommands) to High(CHamCommands) do
    FButtonManager.SetState(CHamCommands[I],
      SameText(CHamCommands[I], 'HAM' + FHamBand) or
      ((FHamBand = 'CB') and SameText(CHamCommands[I], 'CB')));
end;

procedure TfrmMain.UpdateModeButtons;
var
  FMOnly: Boolean;
begin
  if (FFrequencyHz >= 87500000) and (FFrequencyHz <= 108000000) and
     (not SameText(FMode, 'FM')) and (not FApplyingRemoteStatus) then
    FMode := 'FM';
  FMOnly := IsFMBand;

  { En FM radiodiffusion, seul le bouton FM reste disponible. }
  hsAM.Enabled := not FMOnly;
  hsLSB.Enabled := not FMOnly;
  hsUSB.Enabled := not FMOnly;
  hsCW.Enabled := not FMOnly;
  hsCWR.Enabled := False;

  hsFM.Enabled := True;

  { Curseur visuel : main uniquement pour les boutons actifs. }
  if hsAM.Enabled then hsAM.Cursor := crHandPoint else hsAM.Cursor := crDefault;
  if hsLSB.Enabled then hsLSB.Cursor := crHandPoint else hsLSB.Cursor := crDefault;
  if hsUSB.Enabled then hsUSB.Cursor := crHandPoint else hsUSB.Cursor := crDefault;
  if hsCW.Enabled then hsCW.Cursor := crHandPoint else hsCW.Cursor := crDefault;
  if hsCWR.Enabled then hsCWR.Cursor := crHandPoint else hsCWR.Cursor := crDefault;
  hsFM.Cursor := crHandPoint;

  { Forcer l'affichage FM si nécessaire. }
  if FMOnly and (not FApplyingRemoteStatus) then
    EnforceFMBandMode;

  UpdateModeLamps;
end;

function TfrmMain.IsFrontPanelLocked: Boolean;
begin
  Result := FLocked;
end;

procedure TfrmMain.UpdateLockState;
begin
  if hsLock <> nil then
  begin
    hsLock.Enabled := True;
    hsLock.Cursor := crHandPoint;
  end;

  if FButtonManager <> nil then
    FButtonManager.SetState('LOCK', FLocked);
end;

procedure TfrmMain.UpdateModeLamps;
var
  CanScan: Boolean;
begin
  if FButtonManager = nil then
    Exit;

  CanScan := FPowerOn and IsSpectrumScanBandActive;
  hsScan.Enabled := CanScan;

  if CanScan then
    hsScan.Cursor := crHandPoint
  else
    hsScan.Cursor := crDefault;

  { Si on quitte la FM pendant le scan, arreter le scan natif ATS. }
  if (not CanScan) and FScanEnabled then
  begin
    FScanEnabled := False;
    FScanPaused := False;
    FScanStartTick := 0;
    FScanLastActivityTick := 0;

    if (FATSConnection <> nil) and FATSConnection.IsAlive then
      SendATSCommand(TATSProtocol.ScanStop);
    if frmSpectrum <> nil then
      frmSpectrum.AbortScan('Scan interrompu : bande FM quittée.');
  end;

  FButtonManager.SetState(FMode, True);
  FButtonManager.SetState('SCAN', FScanEnabled);
  UpdateLockState;
end;


procedure TfrmMain.UpdateKnobFrame(const ADirection: Integer);
begin
  if FKnobFrames = nil then
    Exit;

  FKnobFrame := FKnobFrame + Sign(ADirection);
  if FKnobFrame < 0 then
    FKnobFrame := FKnobFrames.Count - 1
  else if FKnobFrame >= FKnobFrames.Count then
    FKnobFrame := 0;

  FKnobFrames.AssignTo(imgKnobMotion, FKnobFrame);
end;

procedure TfrmMain.UpdateSMeterFromRSSI(const ARSSI: Integer);
var
  R: Integer;
  SPoint: Integer;
  TargetLevel: Double;
begin
  R := EnsureRange(ARSSI, 0, 100);

  { Reproduction de la conversion RSSI -> S-meter du firmware ATS.
    Le firmware utilise deux courbes différentes :
    - FM
    - AM / SSB / CW
    SPoint correspond à sa position interne 12..208. }

  if IsFMBand or SameText(FMode, 'FM') then
  begin
    { Conversion FM identique à Smeter() du firmware. }
    if R < 1 then
      SPoint := 36
    else if R <= 2 then
      SPoint := 60
    else if R <= 8 then
      SPoint := 84 + (R - 2) * 2
    else if R <= 14 then
      SPoint := 96 + (R - 8) * 2
    else if R <= 24 then
      SPoint := 108 + (R - 14) * 2
    else if R <= 34 then
      SPoint := 124 + (R - 24) * 2
    else if R <= 44 then
      SPoint := 140 + (R - 34) * 2
    else if R <= 54 then
      SPoint := 156 + (R - 44) * 2
    else if R <= 64 then
      SPoint := 172 + (R - 54) * 2
    else if R <= 74 then
      SPoint := 188 + (R - 64) * 2
    else if R <= 76 then
      SPoint := 204
    else
      SPoint := 208;
  end
  else
  begin
    { Conversion HF identique à Smeter() du firmware. }
    if R <= 1 then
      SPoint := 12
    else if R <= 2 then
      SPoint := 24
    else if R <= 3 then
      SPoint := 36
    else if R <= 4 then
      SPoint := 48
    else if R <= 10 then
      SPoint := 48 + (R - 4) * 2
    else if R <= 16 then
      SPoint := 60 + (R - 10) * 2
    else if R <= 22 then
      SPoint := 72 + (R - 16) * 2
    else if R <= 28 then
      SPoint := 84 + (R - 22) * 2
    else if R <= 34 then
      SPoint := 96 + (R - 28) * 2
    else if R <= 44 then
      SPoint := 108 + (R - 34) * 2
    else if R <= 54 then
      SPoint := 124 + (R - 44) * 2
    else if R <= 64 then
      SPoint := 140 + (R - 54) * 2
    else if R <= 74 then
      SPoint := 156 + (R - 64) * 2
    else if R <= 84 then
      SPoint := 172 + (R - 74) * 2
    else if R <= 94 then
      SPoint := 188 + (R - 84) * 2
    else if R <= 95 then
      SPoint := 204
    else
      SPoint := 208;
  end;

  { Adaptation de l'échelle firmware 12..208
    à l'échelle interne de notre aiguille 0..60. }
  TargetLevel := (SPoint - 12) * 60.0 / (208 - 12);
  TargetLevel := EnsureRange(TargetLevel, 0.0, 60.0);

  { Lissage léger seulement, pour éviter les tremblements sans fausser
    sensiblement la valeur affichée. }
  if FSmoothedMeterLevel = 0 then
    FSmoothedMeterLevel := TargetLevel
  else
    FSmoothedMeterLevel :=
      (FSmoothedMeterLevel * 0.35) + (TargetLevel * 0.65);

  FMeterLevel := Round(FSmoothedMeterLevel);

  if FPowerOn then
    SetMeterLevel(FMeterLevel)
  else
  begin
    FSmoothedMeterLevel := 0;
    SetMeterLevel(0);
  end;
end;


procedure TfrmMain.SetMeterLevel(const ALevel: Integer);
const
  CAngleMin = -48.366;
  CAngleMax = 50.377;
var
  L: Integer;
begin
  L := EnsureRange(ALevel, 0, 60);
  FMeterLevel := L;

  if FNeedle <> nil then
    FNeedle.Angle := CAngleMin +
      (CAngleMax - CAngleMin) * (L / 60.0);
end;

procedure TfrmMain.SmallKnobChanged(Sender: TObject);
begin
  if IsFrontPanelLocked then
    Exit;

  if (FSmallKnobs[0] <> nil) and (Sender = FSmallKnobs[0]) then
  begin
    FVolume := FSmallKnobs[0].Value;

    if (not FApplyingRemoteStatus) and FPowerOn and
       (FATSConnection <> nil) and
       FATSConnection.IsAlive then
      SendATSCommand(
        TATSProtocol.SetVolume(FVolume)
      );
  end
  else if (FSmallKnobs[1] <> nil) and (Sender = FSmallKnobs[1]) then
  begin
    FSquelch := FSmallKnobs[1].Value;

    if (not FApplyingRemoteStatus) and FPowerOn and
       (FATSConnection <> nil) and
       FATSConnection.IsAlive then
      SendATSCommand(
        TATSProtocol.SetSquelch(FSquelch)
      );
  end
  else if (FSmallKnobs[2] <> nil) and (Sender = FSmallKnobs[2]) then
  begin
    FBFO := FSmallKnobs[2].Value;
    UpdateBFOText;

    if (not FApplyingRemoteStatus) and FPowerOn and
       (FATSConnection <> nil) and
       FATSConnection.IsAlive then
      SendATSCommand(
        TATSProtocol.SetBFO(FBFO)
      );
  end
  else if (FSmallKnobs[3] <> nil) and (Sender = FSmallKnobs[3]) then
    FRFGain := FSmallKnobs[3].Value
  else if (FSmallKnobs[4] <> nil) and (Sender = FSmallKnobs[4]) then
    FAFGain := FSmallKnobs[4].Value
  else if (FSmallKnobs[5] <> nil) and (Sender = FSmallKnobs[5]) then
    FClarifier := FSmallKnobs[5].Value;

  UpdateKnobValues;
end;


procedure TfrmMain.UpdateBFOText;
begin
  if txtBfo = nil then
    Exit;

  if FBFO > 0 then
    txtBfo.Text := '+' + IntToStr(FBFO)
  else
    txtBfo.Text := IntToStr(FBFO);
end;

procedure TfrmMain.UpdateKnobValues;
begin
  lblStatusDynamic.Caption := Format(
    'VOL=%d  SQL=%d  BFO=%+d Hz  RF=%d  AF=%d  CLAR=%d Hz',
    [FVolume, FSquelch, FBFO, FRFGain, FAFGain, FClarifier]
  );
  UpdateBFOText;
end;


procedure TfrmMain.tmrVisualsTimer(Sender: TObject);
var
  Glow0, Glow1, Glow2: Integer;
begin
if FTubeGlowFrames <> nil then
  begin
    Inc(FGlowPhase);
    Glow0 := 1;
    Glow1 := 1;
    Glow2 := 1;

    if (FGlowPhase mod 17) = 0 then Glow0 := Random(3);
    if (FGlowPhase mod 23) = 0 then Glow1 := Random(3);
    if (FGlowPhase mod 29) = 0 then Glow2 := Random(3);

    FTubeGlowFrames.AssignTo(imgTubeGlow0, Glow0);
    FTubeGlowFrames.AssignTo(imgTubeGlow1, Glow1);
    FTubeGlowFrames.AssignTo(imgTubeGlow2, Glow2);

    imgTubeGlow0.BringToFront;
    imgTubeGlow1.BringToFront;
    imgTubeGlow2.BringToFront;
  end;

  { Défilement du RadioText RDS }
  UpdateRDSRTScroll;
end;

procedure TfrmMain.GetActiveBandLimits(out AMinHz, AMaxHz: Int64);
begin
  AMinHz := CMinFrequencyHz;
  AMaxHz := CMaxFrequencyHz;
  if FHasCurrentHamBand then
  begin
    AMinHz := CHamMin[FCurrentHamBand];
    AMaxHz := CHamMax[FCurrentHamBand];
  end
  else if FInRadioBand and FHasCurrentRadioBand then
  begin
    AMinHz := CRadioMin[FCurrentRadioBand];
    AMaxHz := CRadioMax[FCurrentRadioBand];
  end;
end;

procedure TfrmMain.SelectBandForDirectFrequency(const AFrequencyHz: Int64);
const
  CDefaultHamMode: array[THamBandIndex] of string = (
    'LSB', 'LSB', 'LSB', 'USB', 'USB',
    'USB', 'USB', 'USB', 'AM'
  );
  CHamBandName: array[THamBandIndex] of string = (
    '160', '80', '40', '20', '17', '15', '12', '10', 'CB'
  );
  CRadioDefaultMode: array[TRadioBandIndex] of string = (
    'AM', 'AM', 'AM', 'FM'
  );
  CHamCommands: array[0..8] of string = (
    'HAM160', 'HAM80', 'HAM40', 'HAM20', 'HAM17',
    'HAM15', 'HAM12', 'HAM10', 'CB'
  );
var
  HamBand: THamBandIndex;
  RadioBand: TRadioBandIndex;
  I: Integer;
  Found: Boolean;
begin
  SaveCurrentHamBandMemory;
  SaveCurrentRadioBandMemory;
  FFrequencyHz := AFrequencyHz;

  Found := False;
  for HamBand := Low(THamBandIndex) to High(THamBandIndex) do
  begin
    if (AFrequencyHz >= CHamMin[HamBand]) and
       (AFrequencyHz <= CHamMax[HamBand]) then
    begin
      FCurrentHamBand := HamBand;
      FHamBand := CHamBandName[HamBand];
      FHasCurrentHamBand := True;
      FHasCurrentRadioBand := False;
      FInRadioBand := False;
      if FHamMemories[HamBand].Initialized then
        FMode := FHamMemories[HamBand].Mode
      else
        FMode := CDefaultHamMode[HamBand];
      ApplyHamModeRules(HamBand);
      UpdateHamButtons;
      Found := True;
      Break;
    end;
  end;

  if not Found then
  begin
    for RadioBand := Low(TRadioBandIndex) to High(TRadioBandIndex) do
    begin
      if (AFrequencyHz >= CRadioMin[RadioBand]) and
         (AFrequencyHz <= CRadioMax[RadioBand]) then
      begin
        FCurrentRadioBand := RadioBand;
        FHasCurrentRadioBand := True;
        FHasCurrentHamBand := False;
        FInRadioBand := True;
        if FRadioMemories[RadioBand].Initialized then
          FMode := FRadioMemories[RadioBand].Mode
        else
          FMode := CRadioDefaultMode[RadioBand];
        ApplyRadioModeRules(RadioBand);
        UpdateRadioButtons;
        Found := True;
        Break;
      end;
    end;
  end;

  if not Found then
  begin
    { Fréquence prise en charge par le tuner mais sans bouton de bande dédié. }
    FHasCurrentHamBand := False;
    FHasCurrentRadioBand := False;
    FInRadioBand := False;
    FHamBand := '';
    FMode := 'AM';
    if FButtonManager <> nil then
    begin
      for I := Low(CHamCommands) to High(CHamCommands) do
        FButtonManager.SetState(CHamCommands[I], False);
      FButtonManager.SetState('RADIO_LW', False);
      FButtonManager.SetState('RADIO_MW', False);
      FButtonManager.SetState('RADIO_SW', False);
    end;
    UpdateModeButtons;
  end;
end;

procedure TfrmMain.SyncBandFromStatus(const ABand: string);
var B: string;
begin
  B := UpperCase(Trim(ABand));
  if B = '' then Exit;
  if (B='160') or (B='160M') or (B='160 M') then begin FCurrentHamBand:=hb160m; FHamBand:='160'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='80') or (B='80M') or (B='80 M') then begin FCurrentHamBand:=hb80m; FHamBand:='80'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='40') or (B='40M') or (B='40 M') then begin FCurrentHamBand:=hb40m; FHamBand:='40'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='20') or (B='20M') or (B='20 M') then begin FCurrentHamBand:=hb20m; FHamBand:='20'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='17') or (B='17M') or (B='17 M') or (B='16M') then begin FCurrentHamBand:=hb17m; FHamBand:='17'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='15') or (B='15M') or (B='15 M') or (B='14M') then begin FCurrentHamBand:=hb15m; FHamBand:='15'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='12') or (B='12M') or (B='12 M') then begin FCurrentHamBand:=hb12m; FHamBand:='12'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='10') or (B='10M') or (B='10 M') then begin FCurrentHamBand:=hb10m; FHamBand:='10'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if B='CB' then begin FCurrentHamBand:=hbCB; FHamBand:='CB'; FHasCurrentHamBand:=True; FHasCurrentRadioBand:=False; FInRadioBand:=False; UpdateHamButtons; end
  else if (B='LW') or (B='LONGWAVE') then begin FCurrentRadioBand:=rbLW; FHasCurrentRadioBand:=True; FHasCurrentHamBand:=False; FInRadioBand:=True; UpdateRadioButtons; end
  else if (B='MW') or (B='MEDIUMWAVE') or (B='AM') then begin FCurrentRadioBand:=rbMW; FHasCurrentRadioBand:=True; FHasCurrentHamBand:=False; FInRadioBand:=True; UpdateRadioButtons; end
  else if (B='SW') or (B='SHORTWAVE') then begin FCurrentRadioBand:=rbSW; FHasCurrentRadioBand:=True; FHasCurrentHamBand:=False; FInRadioBand:=True; UpdateRadioButtons; end
  else if B='FM' then begin FCurrentRadioBand:=rbFM; FHasCurrentRadioBand:=True; FHasCurrentHamBand:=False; FInRadioBand:=True; UpdateRadioButtons; end;
end;

function TfrmMain.CBFrequencyForPosition(
  const APosition: Integer): Int64;
var
  Position, BandOffset, Channel: Integer;
begin
  Position := EnsureRange(APosition, 1, CCBTotalPositions);
  BandOffset := ((Position - 1) div CCBChannelsPerBand) - 1;
  Channel := ((Position - 1) mod CCBChannelsPerBand) + 1;
  Result := CCBChannelFrequenciesHz[Channel] +
    (Int64(BandOffset) * CCBBandOffsetHz);
end;

function TfrmMain.CBPositionForFrequency(
  const AFrequencyHz: Int64): Integer;
var
  Position: Integer;
  BestDifference, Difference: Int64;
begin
  Result := 1;
  BestDifference := Abs(AFrequencyHz - CBFrequencyForPosition(Result));
  for Position := 2 to CCBTotalPositions do
  begin
    Difference := Abs(AFrequencyHz - CBFrequencyForPosition(Position));
    if Difference < BestDifference then
    begin
      BestDifference := Difference;
      Result := Position;
    end;
  end;
end;

function TfrmMain.IsCBActive: Boolean;
begin
  Result := FHasCurrentHamBand and (FCurrentHamBand = hbCB);
end;
procedure TfrmMain.ChangeFrequency(const Delta: Int64);
var
  MinHz, MaxHz: Int64;
  CBPosition: Integer;
begin
  UpdateKnobFrame(Sign(Delta));
  GetActiveBandLimits(MinHz, MaxHz);
  if IsCBActive and (Delta <> 0) then
  begin
    CBPosition := CBPositionForFrequency(FFrequencyHz);
    CBPosition := EnsureRange(CBPosition + Sign(Delta),
      1, CCBTotalPositions);
    FFrequencyHz := CBFrequencyForPosition(CBPosition);
  end
  else
    FFrequencyHz := EnsureRange(FFrequencyHz + Delta, MinHz, MaxHz);

  EnforceFMBandMode;
  UpdateModeButtons;
  if FHasCurrentHamBand then
    ApplyHamModeRules(FCurrentHamBand)
  else if FInRadioBand and FHasCurrentRadioBand then
    ApplyRadioModeRules(FCurrentRadioBand);

  UpdateDisplay;

  if (not FApplyingRemoteStatus) and FPowerOn and
     (FATSConnection <> nil) and FATSConnection.IsAlive then
    SendATSCommand(TATSProtocol.SetFrequencyKHz(FFrequencyHz div 1000));

  SaveCurrentHamBandMemory;
  if FInRadioBand then
    SaveCurrentRadioBandMemory;
end;


procedure TfrmMain.UpdateNixieDecimal(const ADigitsBeforePoint: Integer);
var
  RefImage: TImage;
begin
  if FNixieDecimal = nil then
    Exit;

  case ADigitsBeforePoint of
    3: RefImage := imgDigit2;
    5: RefImage := imgDigit4;
  else
    RefImage := imgDigit2;
  end;

  { Le point chevauche légèrement le bord inférieur droit du tube précédent,
    car l'espace physique entre deux tubes est très faible. }
  FNixieDecimal.Left := RefImage.Left + RefImage.Width - 4;
  FNixieDecimal.Top := RefImage.Top + RefImage.Height - 20;
  FNixieDecimal.Visible := FPowerOn;
  FNixieDecimal.BringToFront;
end;


procedure TfrmMain.UpdateNixieDisplay;
var
  S: string;
  Images: array[0..7] of TImage;
  I, Digit: Integer;
  DisplayValue: Int64;
  DigitsBeforePoint: Integer;
begin
  Images[0] := imgDigit0;
  Images[1] := imgDigit1;
  Images[2] := imgDigit2;
  Images[3] := imgDigit3;
  Images[4] := imgDigit4;
  Images[5] := imgDigit5;
  Images[6] := imgDigit6;
  Images[7] := imgDigit7;

  { Affichage MHz sur 8 tubes :
      < 100 MHz  : DDD.ddddd  -> 007.12300 / 014.12300 / 087.50000
      >=100 MHz  : DDDDD.ddd  -> 00105.400 / 00108.000
    Le point décimal est un contrôle séparé ; S ne contient donc que 8 chiffres. }
  if FFrequencyHz >= 100000000 then
  begin
    DisplayValue := FFrequencyHz div 1000;  { précision 1 kHz }
    DigitsBeforePoint := 5;
  end
  else
  begin
    DisplayValue := FFrequencyHz div 10;    { précision 10 Hz }
    DigitsBeforePoint := 3;
  end;

  S := Format('%.8d', [DisplayValue]);

  if Length(FLastNixieText) <> 8 then
    FLastNixieText := '........';

  for I := 0 to 7 do
  begin
    if S[I + 1] = FLastNixieText[I + 1] then
      Continue;

    Digit := Ord(S[I + 1]) - Ord('0');

    if InRange(Digit, 0, 9) then
      Images[I].Picture.Assign(FNixieDigits[Digit]);
  end;

  FLastNixieText := S;
  UpdateNixieDecimal(DigitsBeforePoint);
end;


procedure TfrmMain.UpdateDisplay;
var
  CBBand, CBChannel, CBPosition: Integer;
  CBBandName: string;
begin
  UpdateNixieDisplay;

  if Assigned(LblCanal) then
    LblCanal.Visible := FPowerOn and IsCBActive;
  if IsCBActive then
  begin
    CBPosition := CBPositionForFrequency(FFrequencyHz);
    CBBand := (CBPosition - 1) div CCBChannelsPerBand;
    CBChannel := ((CBPosition - 1) mod CCBChannelsPerBand) + 1;
    case CBBand of
      0: CBBandName := 'INF';
      2: CBBandName := 'SUP';
    else
      CBBandName := 'CB';
    end;
    if Assigned(LblCanal) then
      LblCanal.Caption := Format('%s %.2d', [CBBandName, CBChannel]);
    lblStatusDynamic.Caption := Format(
      'ATS LAB v1.1.1   CONNECTED   %s   %s kHz',
      [FMode, FormatFloat('#,##0', FFrequencyHz div 1000)]
    );
  end
  else
  begin
    if Assigned(LblCanal) then
      LblCanal.Caption := '';
    lblStatusDynamic.Caption := Format(
      'ATS LAB v1.1.1   CONNECTED   %s   %s kHz',
      [FMode, FormatFloat('#,##0', FFrequencyHz div 1000)]
    );
  end;

  if FScanEnabled then
    lblStatusDynamic.Caption := lblStatusDynamic.Caption + '   SCAN';
end;
function TfrmMain.IsFMBand: Boolean;
begin
  { Bande FM radiodiffusion : 87,5 à 108 MHz. }
  Result :=
    (FFrequencyHz >= 87500000) and
    (FFrequencyHz <= 108000000);
end;

function TfrmMain.IsBroadcastFMActive: Boolean;
begin
  Result :=
    FInRadioBand and
    FHasCurrentRadioBand and
    (FCurrentRadioBand = rbFM) and
    IsFMBand and
    SameText(FMode, 'FM');
end;

function TfrmMain.IsSpectrumScanBandActive: Boolean;
begin
  Result := IsBroadcastFMActive or
    (FHasCurrentHamBand and
     (FCurrentHamBand in [hb40m, hb20m, hb15m]));
end;

procedure TfrmMain.EnforceFMBandMode;
begin
  if not IsFMBand then
    Exit;

  if not SameText(FMode, 'FM') then
  begin
    FMode := 'FM';
    UpdateModeImage;
    UpdateModeLamps;
    UpdateDisplay;

    if FPowerOn and (FATSConnection <> nil) and
       FATSConnection.IsAlive then
      SendATSCommand(
        TATSProtocol.SetMode(atmFM)
      );
  end;
end;

procedure TfrmMain.SetModeByIndex(const AIndex: Integer);
const
  Modes: array[0..5] of string = ('AM', 'FM', 'USB', 'LSB', 'CW', 'CW-R');
begin
  if AIndex = 5 then
    Exit;

  if IsFMBand and (AIndex <> 1) then
    Exit;

  if IsFMBand then
  begin
    EnforceFMBandMode;
    Exit;
  end;

  if InRange(AIndex, Low(Modes), High(Modes)) then
  begin
    FMode := Modes[AIndex];
    UpdateDisplay;
    UpdateModeLamps;
  UpdateModeButtons;
  end;

  if (not FApplyingRemoteStatus) and FPowerOn and
       (FATSConnection <> nil) and
     FATSConnection.IsAlive then
  begin
    SendModeCommand(FMode);
  end;
end;

procedure TfrmMain.ModeHotspotClick(Sender: TObject);
begin
  if IsFrontPanelLocked then
    Exit;

  if Sender is TImage then
    SetModeByIndex(TImage(Sender).Tag);
end;

procedure TfrmMain.hsStepClick(Sender: TObject);
begin
  case FStepHz of
    1000: FStepHz := 10000;
    10000: FStepHz := 25000;
    25000: FStepHz := 100000;
  else
    FStepHz := 1000;
  end;

  UpdateStepImage;
  UpdateDisplay;
end;


procedure TfrmMain.hsLockClick(Sender: TObject);
begin
  FLocked := not FLocked;

  if FButtonManager <> nil then
    FButtonManager.SetState('LOCK', FLocked);

  UpdateLockState;
end;


procedure TfrmMain.SpectrumScanEnded(Sender: TObject);
begin
  if not FScanEnabled then
    Exit;

  if (FATSConnection = nil) or not FATSConnection.IsAlive then
  begin
    AbortSpectrumScan('Scan interrompu : connexion ATS perdue.');
    Exit;
  end;

  { Le scanner natif ATS continue de balayer après SCANEND. Le PC conserve
    donc le scan actif et demandera la prochaine image avec SCANDATA?. }
  FScanStartTick := GetTickCount64;
  FScanLastActivityTick := FScanStartTick;
  if frmSpectrum <> nil then
  begin
    frmSpectrum.ContinueScan;
    if FScanPaused then
      frmSpectrum.SetPaused(True);
  end;
  UpdateModeLamps;
  UpdateDisplay;
end;

function TfrmMain.SpectrumTuneRequested(Sender: TObject;
  AFrequencyKHz: Int64): Boolean;
var
  MinHz, MaxHz: Int64;
begin
  Result := False;
  if IsFrontPanelLocked or not FPowerOn or
     not IsSpectrumScanBandActive or
     (FATSConnection = nil) or not FATSConnection.IsAlive then
    Exit;

  GetActiveBandLimits(MinHz, MaxHz);
  AFrequencyKHz := EnsureRange(AFrequencyKHz,
    MinHz div 1000, MaxHz div 1000);
  if FScanEnabled then
    AbortSpectrumScan('Scan arrêté pour l''accord sur le spectre.');
  FFrequencyHz := AFrequencyKHz * 1000;
  FLocalControlUntil := GetTickCount64 + 1500;
  if IsBroadcastFMActive then
    FMode := 'FM'
  else if FHasCurrentHamBand then
    ApplyHamModeRules(FCurrentHamBand);
  UpdateDisplay;
  UpdateModeImage;
  UpdateModeLamps;
  Result := SendATSCommand(TATSProtocol.SetFrequencyKHz(AFrequencyKHz));
  if not Result then
    Exit;
  SaveCurrentHamBandMemory;
  if FInRadioBand then
    SaveCurrentRadioBandMemory;
end;

procedure TfrmMain.SpectrumFavoriteRequested(Sender: TObject);
begin
  ShowFavorites;
end;

procedure TfrmMain.AbortSpectrumScan(const AReason: string);
begin
  if FScanEnabled and (FATSConnection <> nil) and FATSConnection.IsAlive then
    SendATSCommand(TATSProtocol.ScanStop, False);

  FScanEnabled := False;
  FScanPaused := False;
  FScanStartTick := 0;
  FScanLastActivityTick := 0;

  if frmSpectrum <> nil then
    frmSpectrum.AbortScan(AReason);

  UpdateModeLamps;
  UpdateDisplay;
end;


procedure TfrmMain.SpectrumStopRequest(Sender: TObject);
begin
  if FScanEnabled then
    AbortSpectrumScan('Scan arrêté par l''utilisateur.');
end;

procedure TfrmMain.SpectrumPauseRequest(Sender: TObject);
begin
  if not FScanEnabled or FScanPaused then
    Exit;
  if SendATSCommand(TATSProtocol.ScanPause, False) then
  begin
    FScanPaused := True;
    if frmSpectrum <> nil then
      frmSpectrum.SetPaused(True);
  end;
end;

procedure TfrmMain.SpectrumResumeRequest(Sender: TObject);
begin
  if not FScanEnabled or not FScanPaused then
    Exit;
  if SendATSCommand(TATSProtocol.ScanResume, False) then
  begin
    FScanPaused := False;
    FScanStartTick := GetTickCount64;
    FScanLastActivityTick := FScanStartTick;
    if frmSpectrum <> nil then
      frmSpectrum.SetPaused(False);
  end;
end;

procedure TfrmMain.SpectrumStartRequest(Sender: TObject;
  AStartMHz, AStepKHz: Double);
const
  CScanPointCount = 320;
var
  EndKHz: Double;
  ScanPointCount: Integer;
  StartKHz, MinHz, MaxHz: Int64;
begin
  if IsFrontPanelLocked or not FPowerOn or
     (FATSConnection = nil) or not FATSConnection.IsAlive then
    Exit;

  if FScanEnabled then
    AbortSpectrumScan('Redémarrage du scan.');

  GetActiveBandLimits(MinHz, MaxHz);
  StartKHz := Round(AStartMHz * 1000);

  if IsBroadcastFMActive and
     ((AStepKHz < 10) or (AStepKHz > 1000) or
      (Abs((AStepKHz / 10) - Round(AStepKHz / 10)) > 0.0001)) then
  begin
    MessageDlg('En FM, le pas doit être un multiple de 10 kHz, entre 10 et 1000 kHz.',
      mtError, [mbOK], 0);
    Exit;
  end;
  if (not IsBroadcastFMActive) and
     ((AStepKHz < 1) or (AStepKHz > 100) or
      (Abs(AStepKHz - Round(AStepKHz)) > 0.0001)) then
  begin
    MessageDlg('Sur les bandes HAM, le pas doit être un nombre entier de kHz, entre 1 et 100.',
      mtError, [mbOK], 0);
    Exit;
  end;

  ScanPointCount := Integer(
    Trunc(((MaxHz div 1000) - StartKHz) / AStepKHz) + 1);
  if ScanPointCount > CScanPointCount then
    ScanPointCount := CScanPointCount;
  if ScanPointCount < 2 then
  begin
    MessageDlg('La plage restante est trop courte pour effectuer le scan.',
      mtError, [mbOK], 0);
    Exit;
  end;
  EndKHz := StartKHz + ((ScanPointCount - 1) * AStepKHz);
  if (StartKHz * 1000 < MinHz) or (StartKHz * 1000 > MaxHz) or
     (EndKHz * 1000 > MaxHz + 0.1) then
  begin
    MessageDlg(Format(
      'Plage invalide : le scan irait de %.3f à %.3f MHz.' + sLineBreak +
      'Limites de la bande : %.3f à %.3f MHz.',
      [StartKHz / 1000, EndKHz / 1000,
       MinHz / 1000000, MaxHz / 1000000]), mtError, [mbOK], 0);
    Exit;
  end;

  if FRDSEnabledOnATS then
  begin
    SendATSCommand(TATSProtocol.SetRDS(False), False);
    FRDSEnabledOnATS := False;
    ClearRDSDisplay;
  end;

  if not SendATSCommand(TATSProtocol.ScanStartAt(StartKHz, AStepKHz), False) then
    Exit;

  FScanEnabled := True;
  FScanPaused := False;
  FScanStartTick := GetTickCount64;
  FScanLastActivityTick := FScanStartTick;
  if frmSpectrum <> nil then
    frmSpectrum.BeginScan;
  UpdateModeLamps;
  UpdateDisplay;
end;


procedure TfrmMain.hsScanClick(Sender: TObject);
var
  StartMHz, StepKHz: Double;
  MinHz, MaxHz: Int64;
begin
  if IsFrontPanelLocked then
    Exit;

  if not FPowerOn then
    Exit;

  if (FATSConnection = nil) or not FATSConnection.IsAlive then
    Exit;

  if FScanEnabled then
  begin
    if frmSpectrum <> nil then
    begin
      frmSpectrum.Show;
      frmSpectrum.BringToFront;
    end;
    Exit;
  end;

  GetActiveBandLimits(MinHz, MaxHz);
  if IsBroadcastFMActive then
  begin
    StartMHz := FFrequencyHz / 1000000;
    StepKHz := 10;
  end
  else
  begin
    StartMHz := MinHz / 1000000;
    StepKHz := 1;
  end;
  if frmSpectrum <> nil then
    frmSpectrum.PrepareScan(StartMHz, StepKHz);
end;

procedure TfrmMain.hsTuningMouseWheel(Sender: TObject;
  Shift: TShiftState; WheelDelta: Integer; MousePos: TPoint;
  var Handled: Boolean);
begin
  if WheelDelta > 0 then
    ChangeFrequency(FStepHz)
  else if WheelDelta < 0 then
    ChangeFrequency(-FStepHz);

  Handled := True;
end;

procedure TfrmMain.hsTuningMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  P: TPoint;
begin
  if IsFrontPanelLocked then
    Exit;

  if Button <> mbLeft then
    Exit;

  FDragging := True;
  FTuningMoved := False;
  GetCursorPos(P);
  FLastMouseY := P.Y;
end;

procedure TfrmMain.hsTuningMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
var
  P: TPoint;
  DeltaY: Integer;
begin
  if IsFrontPanelLocked then
    Exit;

  if not FDragging then
    Exit;

  GetCursorPos(P);
  DeltaY := FLastMouseY - P.Y;

  if Abs(DeltaY) >= 2 then
  begin
    ChangeFrequency(Sign(DeltaY) * FStepHz);
    FTuningMoved := True;
    FLastMouseY := P.Y;
  end;
end;

procedure TfrmMain.hsTuningMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  if Button = mbLeft then
  begin
    FDragging := False;
    if not FTuningMoved then
      ChangeFrequency(FStepHz);
  end;
end;

procedure TfrmMain.FormMouseWheel(Sender: TObject;
  Shift: TShiftState; WheelDelta: Integer; MousePos: TPoint;
  var Handled: Boolean);
begin
  { La roulette globale est volontairement désactivée.
    La fréquence ne change que sur hsTuning. }
  Handled := False;
end;

procedure TfrmMain.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key = VK_F1 then
  begin
    ShowAbout;
    Key := 0;
    Exit;
  end;

  if IsFrontPanelLocked then
    Exit;

  case Key of
    VK_ESCAPE:
      Close;
    VK_LEFT, VK_DOWN:
      ChangeFrequency(-FStepHz);
    VK_RIGHT, VK_UP:
      ChangeFrequency(FStepHz);
  end;
end;

procedure TfrmMain.UpdatePowerState;
begin
  ion_off.Visible := not FPowerOn;

  imgTube0.Visible := FPowerOn;
  imgDigit0.Visible := FPowerOn;
  imgGlass0.Visible := FPowerOn;
  imgTube1.Visible := FPowerOn;
  imgDigit1.Visible := FPowerOn;
  imgGlass1.Visible := FPowerOn;
  imgTube2.Visible := FPowerOn;
  imgDigit2.Visible := FPowerOn;
  imgGlass2.Visible := FPowerOn;
  imgTube3.Visible := FPowerOn;
  imgDigit3.Visible := FPowerOn;
  imgGlass3.Visible := FPowerOn;
  imgTube4.Visible := FPowerOn;
  imgDigit4.Visible := FPowerOn;
  imgGlass4.Visible := FPowerOn;
  imgTube5.Visible := FPowerOn;
  imgDigit5.Visible := FPowerOn;
  imgGlass5.Visible := FPowerOn;
  imgTube6.Visible := FPowerOn;
  imgDigit6.Visible := FPowerOn;
  imgGlass6.Visible := FPowerOn;
  imgTube7.Visible := FPowerOn;
  imgDigit7.Visible := FPowerOn;
  imgGlass7.Visible := FPowerOn;

  imgTubeGlow0.Visible := FPowerOn;
  imgTubeGlow1.Visible := FPowerOn;
  imgTubeGlow2.Visible := FPowerOn;

  pbMeterNeedle.Visible := True;
  imode.Visible := FPowerOn;

  hsAM.Visible := FPowerOn;
  hsFM.Visible := FPowerOn;
  hsUSB.Visible := FPowerOn;
  hsLSB.Visible := FPowerOn;
  hsCW.Visible := FPowerOn;
  hsCWR.Visible := FPowerOn;
  hsScan.Visible := FPowerOn;
  hsMem.Visible := FPowerOn;
  hsBand.Visible := FPowerOn;
  hsFilter.Visible := FPowerOn;
  hsNB.Visible := FPowerOn;
  hsNR.Visible := FPowerOn;
  hsAGC.Visible := FPowerOn;
  hsNotch.Visible := FPowerOn;
  hsLock.Visible := FPowerOn;

  hsHam160.Visible := FPowerOn;
  hsHam80.Visible := FPowerOn;
  hsHam40.Visible := FPowerOn;
  hsHam20.Visible := FPowerOn;
  hsHam17.Visible := FPowerOn;
  hsHam15.Visible := FPowerOn;
  hsHam12.Visible := FPowerOn;
  hsHam10.Visible := FPowerOn;
  hsHamCB.Visible := FPowerOn;
  if Assigned(HsCB) then
    HsCB.Visible := FPowerOn;
  if Assigned(LblCanal) then
    LblCanal.Visible := FPowerOn and IsCBActive;

  if Assigned(tmrVisuals) then
    tmrVisuals.Enabled := FPowerOn;

  hsPower.Visible := True;
  hsPower.BringToFront;

  if FPowerOn then
  begin
    UpdateNixieDisplay;
    UpdateModeImage;
    UpdateStepImage;
    UpdateModeLamps;
    UpdateHamButtons;
  if FNixieDecimal <> nil then
    FNixieDecimal.Visible := FPowerOn;
end;

  if FPowerOn then
    SetMeterLevel(FMeterLevel)
  else
  begin
    FSmoothedMeterLevel := 0;
    SetMeterLevel(0);
  end;

end;

procedure TfrmMain.ApplyATSStatus(const AStatus: TATSStatus);
var
  NewMode: string;
  ReceivedMode: string;
  NeedModeCorrection: Boolean;
  MinHz, MaxHz, RemoteFrequencyHz: Int64;
begin
  if not AStatus.Valid then
    Exit;

  if AStatus.HasRSSI then
    UpdateSMeterFromRSSI(AStatus.RSSI);

  if GetTickCount64 < FLocalControlUntil then
    Exit;

  { Pendant le balayage, STATUS? décrit la fréquence temporaire du tuner.
    Ne jamais l'utiliser comme fréquence, bande ou mode radio mémorisé. }
  if FScanEnabled then
  begin
    FApplyingRemoteStatus := True;
    try
      if AStatus.HasVolume then
        FVolume := EnsureRange(AStatus.Volume, 0, 100);
      if AStatus.HasSquelch then
        FSquelch := EnsureRange(AStatus.Squelch, 0, 100);
      if AStatus.HasBFO then
        FBFO := EnsureRange(AStatus.BFO, -3000, 3000);
      UpdateBFOText;
      UpdateKnobValues;
    finally
      FApplyingRemoteStatus := False;
    end;
    Exit;
  end;

  if AStatus.HasBand then
    SyncBandFromStatus(AStatus.Band);

  if AStatus.HasFrequency then
  begin
    if AStatus.FrequencyKHz = FLastRemoteFrequencyKHz then
      Inc(FRemoteFrequencyConfirmCount)
    else
    begin
      FLastRemoteFrequencyKHz := AStatus.FrequencyKHz;
      FRemoteFrequencyConfirmCount := 1;
    end;

    if FRemoteFrequencyConfirmCount >= 2 then
    begin
      RemoteFrequencyHz := AStatus.FrequencyKHz * 1000;
      GetActiveBandLimits(MinHz, MaxHz);
      if (RemoteFrequencyHz >= MinHz) and (RemoteFrequencyHz <= MaxHz) then
        FFrequencyHz := RemoteFrequencyHz;
    end;
  end;

  NewMode := '';
  if AStatus.HasMode then
  begin
    NewMode := UpperCase(Trim(AStatus.Mode));
    if NewMode = 'CWR' then
      NewMode := 'CW-R';

    if NewMode = FLastRemoteMode then
      Inc(FRemoteModeConfirmCount)
    else
    begin
      FLastRemoteMode := NewMode;
      FRemoteModeConfirmCount := 1;
    end;
  end;

  ReceivedMode := NewMode;
  NeedModeCorrection := False;

  FApplyingRemoteStatus := True;
  try
    if (FRemoteModeConfirmCount >= 2) and (NewMode <> '') and
       (not SameText(FMode, NewMode)) then
      FMode := NewMode;

    if AStatus.HasVolume then
      FVolume := EnsureRange(AStatus.Volume, 0, 100);
    if AStatus.HasSquelch then
      FSquelch := EnsureRange(AStatus.Squelch, 0, 100);
    if AStatus.HasBFO then
      FBFO := EnsureRange(AStatus.BFO, -3000, 3000);

    if FSmallKnobs[0] <> nil then FSmallKnobs[0].Value := FVolume;
    if FSmallKnobs[1] <> nil then FSmallKnobs[1].Value := FSquelch;
    if FSmallKnobs[2] <> nil then FSmallKnobs[2].Value := FBFO;

    if FHasCurrentHamBand then
      ApplyHamModeRules(FCurrentHamBand)
    else if FInRadioBand and FHasCurrentRadioBand then
      ApplyRadioModeRules(FCurrentRadioBand);

    { Les règles de bande peuvent corriger FMode. Rafraîchir seulement après
      leur application pour éviter un état visuel transitoire ou incohérent. }
    UpdateBFOText;
    UpdateDisplay;
    UpdateModeImage;
    UpdateModeButtons;

    if (FRemoteModeConfirmCount >= 2) and (ReceivedMode <> '') and
       (not SameText(FMode, ReceivedMode)) then
      NeedModeCorrection := True;

    UpdateKnobValues;

    { Un réglage effectué directement sur l'ATS devient la nouvelle mémoire
      de la bande, sans commande de retour pendant cette phase. }
    if (FRemoteFrequencyConfirmCount >= 2) and
       (FRemoteModeConfirmCount >= 2) then
    begin
      GetActiveBandLimits(MinHz, MaxHz);
      if (FFrequencyHz >= MinHz) and (FFrequencyHz <= MaxHz) then
      begin
        SaveCurrentHamBandMemory;
        if FInRadioBand then
          SaveCurrentRadioBandMemory;
      end;
    end;
  finally
    FApplyingRemoteStatus := False;
  end;

  { Si l'ATS annonce un mode interdit pour la bande, remettre aussi
    l'appareil dans le mode corrigé afin d'éviter une divergence PC/ATS. }
  if NeedModeCorrection and FPowerOn and
     (FATSConnection <> nil) and FATSConnection.IsAlive then
  begin
    SendModeCommand(FMode);
  end;
end;


function TfrmMain.QueryWiFiIP(out AIP: string; ATimeoutMs: Cardinal): Boolean;
var
  T0, LastQuery: UInt64;
  Line, U, Rest: string;
  P: Integer;
begin
  Result := False;
  AIP := '';
  if (FATSConnection = nil) or not FATSConnection.IsAlive then Exit;
  T0 := GetTickCount64;
  LastQuery := 0;
  repeat
    if (LastQuery = 0) or ((GetTickCount64 - LastQuery) >= 700) then
    begin
      FATSConnection.SendText(AnsiString('WIFI_STATUS?'#10));
      LastQuery := GetTickCount64;
    end;
    Line := '';
    if FATSConnection.ReadLine(Line, 120) then
    begin
      U := UpperCase(Trim(Line));
      if Pos('WIFI,STATE=CONNECTED', U) = 1 then
      begin
        P := Pos('IP=', U);
        if P > 0 then
        begin
          Rest := Trim(Copy(Line, P + 3, MaxInt));
          P := Pos(',', Rest);
          if P > 0 then Rest := Copy(Rest, 1, P - 1);
          Rest := Trim(Rest);
          if Rest <> '' then begin AIP := Rest; Exit(True); end;
        end;
      end;
    end;
  until (GetTickCount64 - T0) >= ATimeoutMs;
end;

function TfrmMain.WaitForPong(ATimeoutMs: Cardinal): Boolean;
var
  StartTick: UInt64;
  ResponseLine: string;
begin
  Result := False;

  if FConnectionBusy and (ATimeoutMs = 0) then
    Exit;

  if (FATSConnection = nil) or not FATSConnection.IsAlive then
    Exit;

  if not FATSConnection.SendText(TATSProtocol.Ping) then
    Exit;

  StartTick := GetTickCount64;

  repeat
    ResponseLine := '';

    if FATSConnection.ReadLine(ResponseLine, 120) and
       TATSProtocol.IsPong(ResponseLine) then
      Exit(True);

  until (GetTickCount64 - StartTick) >= ATimeoutMs;
end;


function TfrmMain.SendATSCommand(const ACommand: AnsiString;
  ABlockRemote: Boolean): Boolean;
begin
  Result :=
    FPowerOn and
    (FATSConnection <> nil) and
    FATSConnection.IsAlive;

  if not Result then
    Exit;

  if ABlockRemote then
  begin
    FLocalControlUntil := GetTickCount64 + 1200;
    FLastRemoteMode := '';
    FRemoteModeConfirmCount := 0;
    FLastRemoteFrequencyKHz := -1;
    FRemoteFrequencyConfirmCount := 0;
  end;

  Result := FATSConnection.SendText(ACommand);
end;

function TfrmMain.SendModeCommand(const AMode: string;
  ABlockRemote: Boolean): Boolean;
begin
  if SameText(AMode, 'AM') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmAM), ABlockRemote)
  else if SameText(AMode, 'FM') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmFM), ABlockRemote)
  else if SameText(AMode, 'USB') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmUSB), ABlockRemote)
  else if SameText(AMode, 'LSB') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmLSB), ABlockRemote)
  else if SameText(AMode, 'CW') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmCW), ABlockRemote)
  else if SameText(AMode, 'CW-R') or SameText(AMode, 'CWR') then
    Result := SendATSCommand(TATSProtocol.SetMode(atmCWR), ABlockRemote)
  else
    Result := False;
end;

procedure TfrmMain.ATSConnectionLog(Sender: TObject;
  const AText: string);
begin
  if Assigned(FSerialMonitor) then
    FSerialMonitor.AddLine(AText);
end;

procedure TfrmMain.ClearRDSDisplay;
begin
  FRDSStation := '';
  FRDSText := '';
  FRDSPI := '';
  FRDSPTY := '';
  FRDSCT := '';
  UpdateRDSDisplay;
end;

procedure TfrmMain.UpdateRDSRTScroll;
const
  CCharPixels = 8;
  CStartPauseTicks = 8;  { environ 1 seconde avec timer 120 ms }
  CEndPauseTicks = 10;
var
  VisibleChars: Integer;
  MaxPos: Integer;
begin
  if lblRDSRT = nil then
    Exit;

  if FRDSText = '' then
  begin
    lblRDSRT.Caption := '---';
    Exit;
  end;

  VisibleChars := lblRDSRT.Width div CCharPixels;
  if VisibleChars < 4 then
    VisibleChars := 4;

  if Length(FRDSText) <= VisibleChars then
  begin
    lblRDSRT.Caption := FRDSText;
    FRTScrollPos := 1;
    Exit;
  end;

  if FRTScrollPos < 1 then
    FRTScrollPos := 1;

  MaxPos := Length(FRDSText) - VisibleChars + 1;
  if MaxPos < 1 then
    MaxPos := 1;

  lblRDSRT.Caption := Copy(FRDSText, FRTScrollPos, VisibleChars);

  Inc(FRTScrollTick);
  if FRTScrollTick < 2 then
    Exit;
  FRTScrollTick := 0;

  { Pause au début }
  if FRTScrollPos = 1 then
  begin
    if FRTScrollDelay < CStartPauseTicks then
    begin
      Inc(FRTScrollDelay);
      Exit;
    end;
    FRTScrollDelay := 0;
  end;

  { Pause à la fin puis retour au début }
  if FRTScrollPos >= MaxPos then
  begin
    if FRTScrollDelay < CEndPauseTicks then
    begin
      Inc(FRTScrollDelay);
      Exit;
    end;

    FRTScrollDelay := 0;
    FRTScrollPos := 1;
    Exit;
  end;

  Inc(FRTScrollPos);
end;

procedure TfrmMain.UpdateRDSDisplay;
begin
  if lblRDSPS <> nil then
  begin
    if FRDSStation <> '' then
      lblRDSPS.Caption := FRDSStation
    else
      lblRDSPS.Caption := '---';
  end;

  if lblRDSRT <> nil then
  begin
    { Ne réinitialiser le défilement que si le RadioText change réellement. }
    if FRDSText <> FRTScrollSource then
    begin
      FRTScrollSource := FRDSText;
      FRTScrollPos := 1;
      FRTScrollDelay := 0;
      FRTScrollTick := 0;
    end;

    UpdateRDSRTScroll;
  end;

  if lblRDSPI <> nil then
  begin
    if FRDSPI <> '' then
      lblRDSPI.Caption := FRDSPI
    else
      lblRDSPI.Caption := '----';
  end;
  if lblRDSPTY <> nil then
  begin
    if FRDSPTY <> '' then
      lblRDSPTY.Caption := FRDSPTY
    else
      lblRDSPTY.Caption := '---';
  end;

  if lblRDSCT <> nil then
  begin
    if FRDSCT <> '' then
      lblRDSCT.Caption :=  FRDSCT
    else
      lblRDSCT.Caption := '--:--';
  end;
end;

procedure TfrmMain.ParseRDSLine(const ALine: string);
var
  L: string;
  P: Integer;
  V: string;
begin
L := Trim(ALine);

  if StartsText('RDSPS,', L) then
  begin
    FRDSStation := Trim(Copy(L, 7, MaxInt));
    UpdateRDSDisplay;
    Exit;
  end;

  if StartsText('RDSRT,', L) then
  begin
    FRDSText := Trim(Copy(L, 7, MaxInt));
    UpdateRDSDisplay;
    Exit;
  end;

  if StartsText('RDSPI,', L) then
  begin
    FRDSPI := Trim(Copy(L, 7, MaxInt));
    UpdateRDSDisplay;
    Exit;
  end;

  if StartsText('RDSPTY,', L) then
  begin
    FRDSPTY := Trim(Copy(L, 8, MaxInt));
    UpdateRDSDisplay;
    Exit;
  end;

  if StartsText('RDSCT,', L) then
  begin
    V := Trim(Copy(L, 7, MaxInt));
    { Le firmware renvoie par ex. "16:29 -02:00".
      Pour l'afficheur CT, on conserve uniquement HH:MM. }
    if Length(V) >= 5 then
      FRDSCT := Copy(V, 1, 5)
    else
      FRDSCT := V;
    UpdateRDSDisplay;
    Exit;
  end;

  { Etat RDS renvoyé par RDS? / RDS=ON }
  if StartsText('RDS,STATE=', L) then
  begin
    V := UpperCase(Trim(Copy(L, 11, MaxInt)));

    if V = 'OFF' then
      ClearRDSDisplay
    else if V = 'NOT_FM' then
      ClearRDSDisplay
    else if V = 'NO_DATA' then
    begin
      { Ne pas effacer une station déjà décodée :
        NO_DATA peut être temporaire entre deux groupes RDS. }
    end;

    Exit;
  end;
end;


procedure TfrmMain.PollATSStatus;
var
  ResponseLine: string;
  ParsedStatus: TATSStatus;
  StartTick, CurrentTick: UInt64;
  L: string;
begin
  if (FATSConnection = nil) or not FATSConnection.IsAlive then
    Exit;

  if FScanEnabled and not FScanPaused then
  begin
    CurrentTick := GetTickCount64;
    if (frmSpectrum = nil) then
    begin
      AbortSpectrumScan('Scan interrompu : fenêtre Spectrum indisponible.');
    end
    else if (not frmSpectrum.Receiving) and
            (CurrentTick - FScanLastActivityTick >= CScanBeginTimeoutMs) then
    begin
      AbortSpectrumScan('Scan refusé ou SCANBEGIN absent après 5 secondes.');
    end
    else if frmSpectrum.Receiving and
            (CurrentTick - FScanLastActivityTick >= CScanInactivityTimeoutMs) then
    begin
      AbortSpectrumScan('Scan interrompu : aucune donnée reçue depuis 30 secondes.');
    end
    else if CurrentTick - FScanStartTick >= CScanTotalTimeoutMs then
    begin
      AbortSpectrumScan('Scan interrompu : durée maximale de 120 secondes dépassée.');
    end;
  end;

  { Gestion automatique du RDS.
    - En FM : RDS=ON une seule fois, puis RDS? à chaque polling.
    - Hors FM : RDS=OFF une seule fois. }
  if IsBroadcastFMActive and not FScanEnabled then
  begin
    if not FRDSEnabledOnATS then
    begin
      if SendATSCommand(TATSProtocol.SetRDS(True), False) then
        FRDSEnabledOnATS := True;
    end;

    if FRDSEnabledOnATS then
      SendATSCommand(AnsiString('RDS?'#10), False);
  end
  else
  begin
    if FRDSEnabledOnATS then
      SendATSCommand(TATSProtocol.SetRDS(False), False);
    FRDSEnabledOnATS := False;
    ClearRDSDisplay;
  end;

  if not SendATSCommand(TATSProtocol.RequestStatus, False) then
    Exit;

  if FScanEnabled then
    SendATSCommand(TATSProtocol.RequestScanData, False);

  StartTick := GetTickCount64;

  repeat
    ResponseLine := '';

    if FATSConnection.ReadLine(ResponseLine, 80) then
    begin
      L := Trim(ResponseLine);

      { Identification du recepteur : reponse a ID? }
      if StartsText('ID,', L) then
      begin
        ParseReceiverId(L);
        Continue;
      end;

      if StartsText('RDS,', L) or
         StartsText('RDSPS,', L) or
         StartsText('RDSRT,', L) or
         StartsText('RDSPI,', L) or
         StartsText('RDSPTY,', L) or
         StartsText('RDSCT,', L) then
      begin
        if IsBroadcastFMActive and not FScanEnabled then
          ParseRDSLine(L);
        Continue;
      end;

      if FScanEnabled and
         (StartsText('SCANERROR', L) or
          StartsText('SCAN,ERROR', L) or
          StartsText('ERR,SCAN', L) or
          StartsText('ERROR,SCAN', L)) then
      begin
        AbortSpectrumScan('Scan refusé par l''ATS : ' + L);
        Continue;
      end;

      { Le premier balayage complet prend plusieurs secondes avec le temps de
        stabilisation du tuner. Ce message maintient le timeout d'activite,
        tandis que le timeout total de 120 secondes reste applicable. }
      if FScanEnabled and
         (SameText('SCAN,STATE=RUNNING', L) or
          SameText('SCAN,STATE=PAUSED', L)) then
      begin
        FScanPaused := SameText('SCAN,STATE=PAUSED', L);
        FScanLastActivityTick := GetTickCount64;
        if frmSpectrum <> nil then
          frmSpectrum.SetPaused(FScanPaused);
        Continue;
      end;

      if FScanEnabled and
         (StartsText('SCANBEGIN,', L) or
          StartsText('SCANDATA,', L) or
          SameText('SCANEND', L)) then
      begin
        if StartsText('SCANBEGIN,', L) then
        begin
          if ContainsText(L, 'STATE=PAUSED') then
            FScanPaused := True
          else if ContainsText(L, 'STATE=RUNNING') then
            FScanPaused := False;
        end;
        FScanLastActivityTick := GetTickCount64;
        if frmSpectrum <> nil then
          frmSpectrum.ProcessScanLine(L);
        Continue;
      end;

      if TATSProtocol.ParseStatus(L, ParsedStatus) then
      begin
        ApplyATSStatus(ParsedStatus);
        { Continuer à vider le buffer : les trames RDS peuvent suivre. }
      end;
    end;

  until (GetTickCount64 - StartTick) >= 450;
end;


procedure TfrmMain.UpdateConnectionInfo;
begin
  if lblConnectionInfo = nil then
    Exit;

  if (FATSConnection = nil) or not FATSConnection.IsAlive then
  begin
     lblConnectionInfo.Font.Color := clred;
     lblConnectionInfo.Caption := 'Connection : NON CONNECTE'
  end

  else
  begin
    lblConnectionInfo.Font.Color := clGreen;

    case FATSConnection.Transport of
      attSerial:
        lblConnectionInfo.Caption := 'Connection : USB / BLUETOOTH';

      attWiFi:
        lblConnectionInfo.Caption := 'Connection : WIFI';
    else
      begin
        lblConnectionInfo.Font.Color := clRed;
        lblConnectionInfo.Caption := 'Connection : NON CONNECTE';
      end;
    end;
  end;
end;


procedure TfrmMain.ConnectionTimer(Sender: TObject);
begin
  UpdateConnectionInfo;
  if not FPowerOn then
    Exit;

  if (FATSConnection = nil) or not FATSConnection.IsAlive then
  begin
    FConnectionTimer.Enabled := False;
    if FScanEnabled then
      AbortSpectrumScan('Scan interrompu : connexion ATS perdue.');
    FRDSEnabledOnATS := False;
    FPowerOn := False;
    UpdatePowerState;
    lblConnectionInfo.Caption := 'NON CONNECTE';
    lblStatusDynamic.Caption := 'CONNEXION ATS PERDUE';
    Exit;
  end;

  { Tant que l'identification n'a pas ete recue, demander ID?.
    PollATSStatus lira et decodera la reponse dans le flux RX general. }
  if FReceiverModel = '' then
    RequestReceiverId;

  PollATSStatus;
end;


procedure TfrmMain.UpdatePowerDisplay;
const
  CPowerFiles: array[0..4] of string = (
    'Data\ion_off.png',
    'Data\Power\ion_off.png',
    'Data\power_off.png',
    'Data\Power\power_off.png',
    'Data\Buttons\ion_off.png'
  );
var
  I: Integer;
  FileName: string;
begin
  if ion_off.Picture.Graphic = nil then
  begin
    for I := Low(CPowerFiles) to High(CPowerFiles) do
    begin
      FileName := TPath.Combine(
        ExtractFilePath(Application.ExeName),
        CPowerFiles[I]
      );

      if FileExists(FileName) then
      begin
        TGraphicsCache.AssignTo(ion_off.Picture, FileName);
        Break;
      end;
    end;
  end;

  { La façade représente l'état ON.
    L'image ion_off est superposée uniquement à l'état OFF. }
  ion_off.Visible := not FPowerOn;
  ion_off.BringToFront;
 // imgPowerToggle.BringToFront;
end;

procedure TfrmMain.ion_offClick(Sender: TObject);
begin
  hsPowerClick(Sender);
end;



procedure TfrmMain.HsCBClick(Sender: TObject);
begin
  if IsFrontPanelLocked then
    Exit;

  SelectHamBand(hbCB);
end;

procedure TfrmMain.hsRadioFMClick(Sender: TObject);
begin
  if IsFrontPanelLocked then
    Exit;

  SelectRadioBand(rbFM);

  if FPowerOn and (FATSConnection <> nil) and FATSConnection.IsAlive then
    FRDSEnabledOnATS :=
      SendATSCommand(TATSProtocol.SetRDS(True), False);
end;

procedure TfrmMain.NixieClick(Sender: TObject);
var
  F: TfrmFrequencyInput;
begin
  if IsFrontPanelLocked then
    Exit;

  F := TfrmFrequencyInput.Create(Self);
  try
    if not F.Execute(FFrequencyHz) then
      Exit;

    if ((F.FrequencyHz > 30000000) and
        (F.FrequencyHz < 87500000)) then
    begin
      MessageDlg(
        'Fréquence non prise en charge par le firmware ATS.' + sLineBreak +
        'Plages disponibles : 100 kHz à 30 MHz et 87,5 à 108 MHz.',
        mtWarning, [mbOK], 0
      );
      Exit;
    end;

    if FScanEnabled then
      AbortSpectrumScan('Scan arrêté par la saisie directe de fréquence.');

    SelectBandForDirectFrequency(F.FrequencyHz);
    FLocalControlUntil := GetTickCount64 + 1500;

    EnforceFMBandMode;
    UpdateModeButtons;

    if FHasCurrentHamBand then
      ApplyHamModeRules(FCurrentHamBand)
    else if FInRadioBand and FHasCurrentRadioBand then
      ApplyRadioModeRules(FCurrentRadioBand);

    UpdateDisplay;
    UpdateModeImage;
    UpdateModeLamps;

    if FPowerOn and (FATSConnection <> nil) and FATSConnection.IsAlive then
    begin
      SendATSCommand(TATSProtocol.SetFrequencyKHz(FFrequencyHz div 1000));
      SendModeCommand(FMode);
    end;

    SaveCurrentHamBandMemory;
    if FInRadioBand then
      SaveCurrentRadioBandMemory;
  finally
    F.Free;
  end;
end;

procedure TfrmMain.hsPowerClick(Sender: TObject);
var
  ConnForm: TfrmSerialConnect;
  WifiForm: TfrmWifiCredentials;
  Ini: TIniFile;
  IniFileName: string;
  SavedSSID: string;
  SavedPassword: string;
  SavedIP: string;
  SavedTcpPort: Integer;
  WifiIP: string;
  PongOK: Boolean;
  DirectWiFiOK: Boolean;
begin
  if FConnectionBusy then
    Exit;

  FConnectionBusy := True;
  try
      if FPowerOn then
      begin
        FConnectionTimer.Enabled := False;
        if FScanEnabled then
          AbortSpectrumScan('Scan arrêté avant la déconnexion.');
        FATSConnection.Disconnect;
        lblConnectionInfo.Font.Color := clRed;
        lblConnectionInfo.Caption := 'Connexion : NON CONNECTE';
        FRDSEnabledOnATS := False;
        FPowerOn := False;
        UpdatePowerState;
        lblStatusDynamic.Caption := 'ATS DECONNECTE';
        Exit;
      end;

      ConnForm := TfrmSerialConnect.Create(Self);
      try
        if ConnForm.ShowModal <> mrOk then
          Exit;

        IniFileName := ChangeFileExt(Application.ExeName, '.ini');

        { =========================================================
          CONNEXION SERIE NORMALE
          ========================================================= }
        if ConnForm.ConnectionChoice in [accSerial, accBluetooth] then
        begin
          if not FATSConnection.Connect(
            ConnForm.SelectedPort,
            ConnForm.SelectedBaud
          ) then
          begin
            MessageDlg(
              IfThen(ConnForm.ConnectionChoice = accBluetooth,
                'Connexion Bluetooth impossible : ', 'Connexion série impossible : ') +
                FATSConnection.LastError,
              mtError, [mbOK], 0
            );
            Exit;
          end;

          PongOK := WaitForPong(1800);

          if not PongOK then
          begin
            MessageDlg(
              IfThen(ConnForm.ConnectionChoice = accBluetooth,
                'Le port COM Bluetooth est ouvert mais l''ATS ne répond pas au PING.',
                'Le port COM est ouvert mais l''ATS ne répond pas au PING.'),
              mtError, [mbOK], 0
            );
            FATSConnection.Disconnect;
            Exit;
          end;

          FRDSEnabledOnATS := False;
          FPowerOn := True;
          FConnectionTimer.Enabled := True;
          UpdatePowerState;

          if ConnForm.ConnectionChoice = accBluetooth then
            lblStatusDynamic.Caption := Format(
              'ATS BLUETOOTH %s - CONNECTE',
              [FATSConnection.PortName]
            )
          else
            lblStatusDynamic.Caption := Format(
              'ATS SERIE %s @ %d - CONNECTE',
              [FATSConnection.PortName, FATSConnection.BaudRate]
            );
          Exit;
        end;

        { =========================================================
          MODE WI-FI :
          1) essayer d'abord directement LastIP:TcpPort depuis l'INI
          ========================================================= }
        SavedSSID := '';
        SavedPassword := '';
        SavedIP := '';
        SavedTcpPort := 3333;

        Ini := TIniFile.Create(IniFileName);
        try
          SavedSSID := DecodeIniSecret(Ini.ReadString('WiFi', 'SSID', ''));
          SavedPassword := DecodeIniSecret(Ini.ReadString('WiFi', 'Password', ''));
          SavedIP := Ini.ReadString('WiFi', 'LastIP', '');
          SavedTcpPort := Ini.ReadInteger('WiFi', 'TcpPort', 3333);
        finally
          Ini.Free;
        end;

        DirectWiFiOK := False;

        if (Trim(SavedSSID) <> '') and
           (Trim(SavedIP) <> '') then
        begin
          lblStatusDynamic.Caption := Format(
            'CONNEXION WIFI DIRECTE %s:%d...',
            [SavedIP, SavedTcpPort]
          );
          lblStatusDynamic.Update;

          if FATSConnection.ConnectTCP(SavedIP, SavedTcpPort) then
          begin
            PongOK := WaitForPong(2000);

            if PongOK then
              DirectWiFiOK := True
            else
              FATSConnection.Disconnect;
          end;
        end;

        if DirectWiFiOK then
        begin
          FRDSEnabledOnATS := False;
          FPowerOn := True;
          FConnectionTimer.Enabled := True;
          UpdatePowerState;

          lblStatusDynamic.Caption := Format(
            'ATS WIFI %s:%d - CONNECTE DIRECTEMENT',
            [SavedIP, SavedTcpPort]
          );
          Exit;
        end;

        { =========================================================
          2) Le Wi-Fi direct a échoué ou n'est pas encore configuré.
             On utilise alors l'USB comme procédure de configuration.
          ========================================================= }
        FATSConnection.Disconnect;

        if ConnForm.SelectedPort = '' then
        begin
          MessageDlg(
            'La connexion Wi-Fi directe a échoué.'#13#10#13#10 +
            'Sélectionne le port COM de l''ATS puis relance la connexion Wi-Fi ' +
            'pour reconfigurer le réseau.',
            mtInformation, [mbOK], 0
          );
          Exit;
        end;

        lblStatusDynamic.Caption := 'WIFI DIRECT INDISPONIBLE - CONNEXION USB...';
        lblStatusDynamic.Update;

        if not FATSConnection.Connect(
          ConnForm.SelectedPort,
          ConnForm.SelectedBaud
        ) then
        begin
          MessageDlg(
            'Connexion USB de secours impossible : ' +
            FATSConnection.LastError,
            mtError, [mbOK], 0
          );
          Exit;
        end;

        PongOK := WaitForPong(1800);

        if not PongOK then
        begin
          MessageDlg(
            'L''ATS ne répond pas au PING sur le port USB.',
            mtError, [mbOK], 0
          );
          FATSConnection.Disconnect;
          Exit;
        end;

        { Popup Wi-Fi : prérempli automatiquement depuis l'INI. }
        WifiForm := TfrmWifiCredentials.Create(Self);
        try
          if not WifiForm.Execute(IniFileName) then
          begin
            FATSConnection.Disconnect;
            Exit;
          end;

          if not FATSConnection.SendText(
            AnsiString(
              'WIFI_SSID=' + Trim(WifiForm.edtSSID.Text) + #10
            )
          ) or
             not FATSConnection.SendText(
            AnsiString(
              'WIFI_PASS=' + WifiForm.edtPassword.Text + #10
            )
          ) or
             not FATSConnection.SendText(
            AnsiString('WIFI_SAVE'#10)
          ) or
             not FATSConnection.SendText(
            AnsiString('WIFI_CONNECT'#10)
          ) then
          begin
            MessageDlg(
              'Erreur pendant l''envoi des paramètres Wi-Fi.',
              mtError, [mbOK], 0
            );
            FATSConnection.Disconnect;
            Exit;
          end;

          if not QueryWiFiIP(WifiIP, 15000) then
          begin
            MessageDlg(
              'L''ATS n''a pas confirmé sa connexion Wi-Fi.'#13#10 +
              'Vérifie le SSID et le mot de passe.',
              mtWarning, [mbOK], 0
            );
            FATSConnection.Disconnect;
            Exit;
          end;

          { Mémoriser la nouvelle IP pour les prochaines connexions
            entièrement sans USB. }
          Ini := TIniFile.Create(IniFileName);
          try
            Ini.WriteString('WiFi', 'LastIP', WifiIP);
            Ini.WriteInteger('WiFi', 'TcpPort', 3333);
            Ini.UpdateFile;
          finally
            Ini.Free;
          end;

        finally
          WifiForm.Free;
        end;

        { =========================================================
          3) Fermeture USB puis TCP avec l'IP nouvellement obtenue.
          ========================================================= }
        FATSConnection.Disconnect;

        lblStatusDynamic.Caption := Format(
          'CONNEXION TCP %s:%d...',
          [WifiIP, 3333]
        );
        lblStatusDynamic.Update;

        if not FATSConnection.ConnectTCP(WifiIP, 3333) then
        begin
          MessageDlg(
            'Connexion TCP impossible : ' + FATSConnection.LastError,
            mtError, [mbOK], 0
          );
          Exit;
        end;

        PongOK := WaitForPong(2000);

        if not PongOK then
        begin
          MessageDlg(
            'TCP ouvert mais l''ATS ne répond pas au PING.',
            mtWarning, [mbOK], 0
          );
          FATSConnection.Disconnect;
          Exit;
        end;

        FRDSEnabledOnATS := False;
        FPowerOn := True;
        FConnectionTimer.Enabled := True;
        UpdatePowerState;

        lblStatusDynamic.Caption := Format(
          'ATS WIFI %s:%d - CONNECTE',
          [WifiIP, 3333]
        );

      finally
        ConnForm.Free;
      end;
  finally
    FConnectionBusy := False;
  end;
end;


end.
