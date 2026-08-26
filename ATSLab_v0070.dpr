program ATSLab_v0070;

uses
  Winapi.Windows,
  Vcl.Forms,
  uSteamVisuals in 'uSteamVisuals.pas',
  uRotatingNeedle in 'uRotatingNeedle.pas',
  uSteamKnob in 'uSteamKnob.pas',
  uSteamButton in 'uSteamButton.pas',
  uSteamButtonManager in 'uSteamButtonManager.pas',
  SerialMonitorUnit in 'SerialMonitorUnit.pas' {frmSerialMonitor},
  ConnectionDiagnosticUnit in 'ConnectionDiagnosticUnit.pas',
  SerialConnectUnit in 'SerialConnectUnit.pas' {frmSerialConnect},
  uATSConnection in 'uATSConnection.pas',
  uATSProtocol in 'uATSProtocol.pas',
  MainUnit in 'MainUnit.pas' {frmMain},
  WifiCredentialsUnit in 'WifiCredentialsUnit.pas' {frmWifiCredentials},
  FrequencyInputUnit in 'FrequencyInputUnit.pas' {frmFrequencyInput},
  AboutUnit in 'AboutUnit.pas' {AboutForm},
  SpectrumUnit in 'SpectrumUnit.pas' {frmSpectrum},
  FavoritesUnit in 'FavoritesUnit.pas' {frmFavorites},
  SplashUnit in 'SplashUnit.pas';

{$R *.res}

var
  Splash: TfrmSplash;
  SplashStart: UInt64;
  SplashElapsed: UInt64;

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.Title := 'ATS LAB v1.1.1';
  Splash := TfrmSplash.CreateNew(nil);
  try
    SplashStart := GetTickCount64;
    Splash.Show;
    Splash.Update;
    Application.CreateForm(TfrmMain, frmMain);
    SplashElapsed := GetTickCount64 - SplashStart;
    if SplashElapsed < 2500 then
      Sleep(Cardinal(2500 - SplashElapsed));
  finally
    Splash.Free;
  end;
  Application.Run;
end.
