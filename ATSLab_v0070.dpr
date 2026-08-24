program ATSLab_v0070;

uses
  Vcl.Forms,
  uSteamVisuals in 'uSteamVisuals.pas',
  uRotatingNeedle in 'uRotatingNeedle.pas',
  uSteamKnob in 'uSteamKnob.pas',
  uSteamButton in 'uSteamButton.pas',
  uSteamButtonManager in 'uSteamButtonManager.pas',
  SerialMonitorUnit in 'SerialMonitorUnit.pas' {frmSerialMonitor},
  SerialConnectUnit in 'SerialConnectUnit.pas' {frmSerialConnect},
  uATSConnection in 'uATSConnection.pas',
  uATSProtocol in 'uATSProtocol.pas',
  MainUnit in 'MainUnit.pas' {frmMain},
  WifiCredentialsUnit in 'WifiCredentialsUnit.pas' {frmWifiCredentials},
  FrequencyInputUnit in 'FrequencyInputUnit.pas' {frmFrequencyInput},
  AboutUnit in 'AboutUnit.pas' {AboutForm},
  SpectrumUnit in 'SpectrumUnit.pas' {frmSpectrum},
  FavoritesUnit in 'FavoritesUnit.pas' {frmFavorites};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.Title := 'ATS LAB v1.1.1';
  Application.CreateForm(TfrmMain, frmMain);
  Application.Run;
end.
