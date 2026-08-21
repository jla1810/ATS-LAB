unit SerialConnectUnit;

interface

uses
  Winapi.Windows, Winapi.Messages,
  System.SysUtils, System.Classes, System.IOUtils, System.Math, System.IniFiles,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.StdCtrls,
  Vcl.ExtCtrls, Vcl.Dialogs,
  uSteamButton, Vcl.Imaging.pngimage, System.UITypes,
  System.Win.Registry,
  uATSConnection, uATSProtocol;

type
  TATSConnectionChoice = (
    accSerial,
    accWiFi,
    accBluetooth
  );

  TfrmSerialConnect = class(TForm)
    imgBackground: TImage;
    imgPowerLamp: TImage;
    imgUSBLamp: TImage;
    imgWifiLamp: TImage;
    lblTitle: TLabel;
    lblPowerName: TLabel;
    lblUSBName: TLabel;
    lblWifiName: TLabel;
    lblPowerState: TLabel;
    lblUSBState: TLabel;
    lblWifiState: TLabel;
    lblPort: TLabel;
    lblStatusTitle: TLabel;
    lblStatusText: TLabel;
    cbPort: TComboBox;
    imgRefresh: TImage;
    lblRefresh: TLabel;
    imgConnect: TImage;
    imgCancel: TImage;
    imgClose: TImage;
    shpPower: TShape;
    shpUSB: TShape;
    shpWifi: TShape;
    lblTransport: TLabel;
    cbTransport: TComboBox;
    edtHost: TEdit;
    edtTcpPort: TEdit;
    edtSSID: TEdit;
    edtWifiPass: TEdit;
    btnWifiConfig: TButton;
    btnClearWifi: TButton;
    lblSSID: TLabel;
    lblWifiPass: TLabel;
    lblHost: TLabel;
    lblTcpPort: TLabel;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure DragWindow(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure imgCloseClick(Sender: TObject);
    procedure imgRefreshClick(Sender: TObject);
    procedure imgConnectClick(Sender: TObject);
    procedure imgCancelClick(Sender: TObject);
    procedure cbTransportChange(Sender: TObject);
    procedure btnWifiConfigClick(Sender: TObject);
    procedure btnClearWifiClick(Sender: TObject);
  private
    FOperationBusy: Boolean;
    FConnectButton: TSteamButton;
    FCancelButton: TSteamButton;
    FSelectedPort: string;
    FSelectedBaud: Integer;
    FConnectionChoice: TATSConnectionChoice;
    FSelectedHost: string;
    FSelectedTcpPort: Integer;
    FWifiConfigRequested: Boolean;
    FSSID: string;
    FWifiPassword: string;
    FConnection: TATSConnection;
    FUSBConnected: Boolean;
    procedure LoadSerialPorts;
    procedure UpdateTransportUI;
    function WaitForPong(ATimeoutMs: Cardinal): Boolean;
    function ConnectUSBNow: Boolean;
    function ConnectWiFiNow: Boolean;
    function WaitForWiFiIP(out AIP: string; ATimeoutMs: Cardinal): Boolean;
public
    property SelectedPort: string read FSelectedPort;
    property SelectedBaud: Integer read FSelectedBaud;
    property ConnectionChoice: TATSConnectionChoice read FConnectionChoice;
    property SelectedHost: string read FSelectedHost;
    property SelectedTcpPort: Integer read FSelectedTcpPort;
    property WifiConfigRequested: Boolean read FWifiConfigRequested;
    property SSID: string read FSSID;
    property WifiPassword: string read FWifiPassword;
    property Connection: TATSConnection read FConnection write FConnection;
  end;

implementation

{$R *.dfm}

procedure TfrmSerialConnect.FormCreate(Sender: TObject);
var
  BasePath: string;
  ButtonPath: string;
begin
  FOperationBusy := False;
  FUSBConnected := False;
  DoubleBuffered := True;

  BasePath := ExtractFilePath(Application.ExeName);

  imgBackground.Picture.LoadFromFile(
    TPath.Combine(BasePath, 'Data\Windows\serial_connect_full_bg.png')
  );

  imgPowerLamp.Picture.LoadFromFile(
    TPath.Combine(BasePath, 'Data\Windows\serial_power_lamp.png')
  );
  imgUSBLamp.Picture.LoadFromFile(
    TPath.Combine(BasePath, 'Data\Windows\serial_usb_lamp.png')
  );
  imgWifiLamp.Picture.LoadFromFile(
    TPath.Combine(BasePath, 'Data\Windows\serial_wifi_lamp.png')
  );

  ButtonPath := TPath.Combine(BasePath, 'Data\Buttons');

  FConnectButton := TSteamButton.Create(
    imgConnect,
    TPath.Combine(ButtonPath, 'serial_connect_off.png'),
    TPath.Combine(ButtonPath, 'serial_connect_on.png'),
    TPath.Combine(ButtonPath, 'serial_connect_pressed.png')
  );

  FCancelButton := TSteamButton.Create(
    imgCancel,
    TPath.Combine(ButtonPath, 'serial_cancel_off.png'),
    TPath.Combine(ButtonPath, 'serial_cancel_on.png'),
    TPath.Combine(ButtonPath, 'serial_cancel_pressed.png')
  );
  cbTransport.Items.Clear;
  cbTransport.Items.Add('SERIE / USB');
  cbTransport.Items.Add('WI-FI / TCP');
  cbTransport.Items.Add('BLUETOOTH / SPP');
  cbTransport.ItemIndex := 0;

  edtHost.Text := 'ats25.local';
  edtTcpPort.Text := '3333';

  FConnectionChoice := accSerial;
  FWifiConfigRequested := False;
  edtSSID.Text := '';
  edtWifiPass.Text := '';

  LoadSerialPorts;
  FSelectedBaud := 115200;
  UpdateTransportUI;

  lblPowerState.Caption := 'Éteint';
  lblUSBState.Caption := 'Inactif';
  lblWifiState.Caption := 'Inactif';
  lblStatusTitle.Caption := 'Recherche du port série...';
  lblStatusText.Caption :=
    'Veuillez sélectionner le port et cliquer sur CONNECTER.';
end;

procedure TfrmSerialConnect.FormDestroy(Sender: TObject);
begin
  FreeAndNil(FConnectButton);
  FreeAndNil(FCancelButton);
end;

procedure TfrmSerialConnect.DragWindow(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  if Button = mbLeft then
  begin
    ReleaseCapture;
    Perform(WM_NCLBUTTONDOWN, HTCAPTION, 0);
  end;
end;

procedure TfrmSerialConnect.LoadSerialPorts;
const
  CSerialCommKey = 'HARDWARE\DEVICEMAP\SERIALCOMM';
var
  Registry: TRegistry;
  ValueNames: TStringList;
  Ports: TStringList;
  ValueName: string;
  PortName: string;
  PreviousPort: string;
begin
  PreviousPort := cbPort.Text;

  ValueNames := TStringList.Create;
  Ports := TStringList.Create;
  Registry := TRegistry.Create(KEY_READ);
  try
    Ports.Sorted := True;
    Ports.Duplicates := dupIgnore;

    Registry.RootKey := HKEY_LOCAL_MACHINE;

    if Registry.OpenKeyReadOnly(CSerialCommKey) then
    begin
      Registry.GetValueNames(ValueNames);

      for ValueName in ValueNames do
      begin
        PortName := Registry.ReadString(ValueName);

        if PortName <> '' then
          Ports.Add(PortName);
      end;

      Registry.CloseKey;
    end;

    cbPort.Items.BeginUpdate;
    try
      cbPort.Items.Assign(Ports);

      if PreviousPort <> '' then
        cbPort.ItemIndex := cbPort.Items.IndexOf(PreviousPort);

      if (cbPort.ItemIndex < 0) and (cbPort.Items.Count > 0) then
        cbPort.ItemIndex := 0;
    finally
      cbPort.Items.EndUpdate;
    end;
  finally
    Registry.Free;
    Ports.Free;
    ValueNames.Free;
  end;

  if cbPort.Items.Count = 0 then
  begin
    lblStatusTitle.Caption := 'Aucun port série détecté';
    lblStatusText.Caption :=
      'Branche l''ATS puis clique sur RAFRAÎCHIR.';
  end
  else
  begin
    lblStatusTitle.Caption := Format(
      '%d port(s) série détecté(s)',
      [cbPort.Items.Count]
    );
    lblStatusText.Caption :=
      'Sélectionne le port puis clique sur CONNECTER.';
  end;
end;





function TfrmSerialConnect.WaitForPong(ATimeoutMs: Cardinal): Boolean;
var T0: UInt64; L: string;
begin
  Result := False;
  if (FConnection=nil) or not FConnection.SendText(TATSProtocol.Ping) then Exit;
  T0 := GetTickCount64;
  repeat
    L := '';
    if FConnection.ReadLine(L,120) and TATSProtocol.IsPong(L) then Exit(True);
    Sleep(2);
  until GetTickCount64-T0 >= ATimeoutMs;
end;

function TfrmSerialConnect.ConnectUSBNow: Boolean;
begin
  Result:=False;
  if FConnection=nil then Exit;
  if cbPort.ItemIndex<0 then begin MessageDlg('Sélectionne le port COM USB.',mtWarning,[mbOK],0); Exit; end;
  FSelectedPort:=cbPort.Text;
  FSelectedBaud := 115200;
  FConnection.Disconnect;
  if not FConnection.Connect(FSelectedPort,FSelectedBaud) then begin
    MessageDlg('Connexion USB impossible : '+FConnection.LastError,mtError,[mbOK],0); Exit; end;
  if not WaitForPong(1800) then begin
    MessageDlg('ATS sans réponse au PING USB.',mtError,[mbOK],0); FConnection.Disconnect; Exit; end;
  FUSBConnected:=True;
  lblUSBState.Caption:='Actif';
  lblStatusTitle.Caption:='USB CONNECTE';
  lblStatusText.Caption:='Sélectionne maintenant WI-FI / TCP.';
  Result:=True;
end;

function TfrmSerialConnect.WaitForWiFiIP(
  out AIP: string; ATimeoutMs: Cardinal): Boolean;
var
  StartTick: UInt64;
  LastQueryTick: UInt64;
  Line: string;
  UpperLine: string;
  P: Integer;
  Rest: string;
begin
  Result := False;
  AIP := '';

  if (FConnection = nil) or not FConnection.IsAlive then
    Exit;

  StartTick := GetTickCount64;
  LastQueryTick := 0;

  repeat
    { Comme dans ats25v15 : demander explicitement l'état Wi-Fi. }
    if (LastQueryTick = 0) or
       ((GetTickCount64 - LastQueryTick) >= 700) then
    begin
      FConnection.SendText(AnsiString('WIFI_STATUS?'#10));
      LastQueryTick := GetTickCount64;
    end;

    Line := '';

    if FConnection.ReadLine(Line, 120) then
    begin
      UpperLine := UpperCase(Trim(Line));

      if Pos('WIFI,STATE=CONNECTED', UpperLine) = 1 then
      begin
        P := Pos('IP=', UpperLine);

        if P > 0 then
        begin
          Rest := Trim(Copy(Line, P + 3, MaxInt));
          P := Pos(',', Rest);

          if P > 0 then
            Rest := Copy(Rest, 1, P - 1);

          Rest := Trim(Rest);

          if Rest <> '' then
          begin
            AIP := Rest;
            Result := True;
            Exit;
          end;
        end;
      end;

      { Afficher la réponse directement dans la fenêtre de connexion. }
      if Pos('WIFI,', UpperLine) = 1 then
      begin
        lblStatusTitle.Caption := 'ÉTAT WI-FI ATS';
        lblStatusText.Caption := Trim(Line);
      end;
    end;

  until (GetTickCount64 - StartTick) >= ATimeoutMs;
end;

function TfrmSerialConnect.ConnectWiFiNow: Boolean;
var
  OK: Boolean;
  WifiIP: string;
begin
  Result := False;

  if not FUSBConnected then
  begin
    MessageDlg(
      'Connecte d''abord l''ATS en USB sans fermer cette fenêtre.',
      mtWarning, [mbOK], 0
    );
    Exit;
  end;

  if (FConnection = nil) or
     (FConnection.Transport <> attSerial) or
     not FConnection.IsAlive then
  begin
    MessageDlg(
      'La liaison USB n''est plus active.',
      mtWarning, [mbOK], 0
    );
    Exit;
  end;

  FSSID := Trim(edtSSID.Text);
  FWifiPassword := edtWifiPass.Text;

  if FSSID = '' then
  begin
    MessageDlg('Le SSID Wi-Fi est vide.', mtWarning, [mbOK], 0);
    edtSSID.SetFocus;
    Exit;
  end;

  if MessageDlg(
       'SSID à envoyer : ' + FSSID + sLineBreak + sLineBreak +
       'Continuer ?',
       mtConfirmation,
       [mbYes, mbNo],
       0
     ) <> mrYes then
    Exit;

  FSelectedTcpPort := StrToIntDef(Trim(edtTcpPort.Text), 3333);

  if FSSID = '' then
  begin
    MessageDlg('Entre le SSID.', mtWarning, [mbOK], 0);
    Exit;
  end;

  lblStatusTitle.Caption := 'CONFIGURATION WI-FI';
  lblStatusText.Caption := 'Envoi SSID / mot de passe via USB...';
  lblStatusTitle.Update;
  lblStatusText.Update;

  OK := True;
  OK := OK and FConnection.SendText(
    AnsiString('WIFI_SSID=' + FSSID + #10)
  );
  OK := OK and FConnection.SendText(
    AnsiString('WIFI_PASS=' + FWifiPassword + #10)
  );
  OK := OK and FConnection.SendText(
    AnsiString('WIFI_SAVE'#10)
  );
  OK := OK and FConnection.SendText(
    AnsiString('WIFI_CONNECT'#10)
  );

  if not OK then
  begin
    MessageDlg(
      'Erreur pendant l''envoi de la configuration Wi-Fi.',
      mtError, [mbOK], 0
    );
    Exit;
  end;

  lblStatusTitle.Caption := 'CONNEXION DE L''ATS AU WI-FI';
  lblStatusText.Caption :=
    'Attente de WIFI,STATE=CONNECTED et de l''adresse IP...';
  lblStatusTitle.Update;
  lblStatusText.Update;

  { IMPORTANT :
    on garde l'USB ouvert et on interroge WIFI_STATUS? jusqu'à récupérer
    l'adresse IPv4 réelle, exactement comme ats25v15. }
  if not WaitForWiFiIP(WifiIP, 15000) then
  begin
    MessageDlg(
      'L''ATS n''a pas confirmé sa connexion Wi-Fi.'#13#10 +
      'Vérifie le SSID et le mot de passe.',
      mtWarning, [mbOK], 0
    );
    Exit;
  end;

  FSelectedHost := WifiIP;
  edtHost.Text := WifiIP;

  lblStatusTitle.Caption := 'WI-FI ATS CONNECTÉ';
  lblStatusText.Caption :=
    Format('IP reçue : %s - basculement vers TCP %d...',
      [FSelectedHost, FSelectedTcpPort]);
  lblStatusTitle.Update;
  lblStatusText.Update;

  { Seulement maintenant on libère l'USB. }
  FConnection.Disconnect;
  FUSBConnected := False;
  lblUSBState.Caption := 'Inactif';

  if not FConnection.ConnectTCP(FSelectedHost, FSelectedTcpPort) then
  begin
    MessageDlg(
      'L''ATS est connecté au Wi-Fi à l''adresse ' + FSelectedHost +
      ', mais la connexion TCP au port ' +
      IntToStr(FSelectedTcpPort) + ' échoue.'#13#10#13#10 +
      FConnection.LastError,
      mtWarning, [mbOK], 0
    );
    Exit;
  end;

  if not WaitForPong(2000) then
  begin
    MessageDlg(
      'TCP connecté à ' + FSelectedHost +
      ' mais l''ATS ne répond pas au PING.',
      mtWarning, [mbOK], 0
    );
    FConnection.Disconnect;
    Exit;
  end;

  lblWifiState.Caption := 'Actif';
  lblStatusTitle.Caption := 'WI-FI CONNECTÉ';
  lblStatusText.Caption :=
    Format('%s:%d - liaison TCP active',
      [FSelectedHost, FSelectedTcpPort]);

  Result := True;
end;


procedure TfrmSerialConnect.UpdateTransportUI;
begin
  cbPort.Visible := True;
  lblPort.Visible := True;
  edtHost.Visible := False;
  edtTcpPort.Visible := False;
  edtSSID.Visible := False;
  edtWifiPass.Visible := False;
  btnWifiConfig.Visible := False;
  btnClearWifi.Visible := False;
  lblSSID.Visible := False;
  lblWifiPass.Visible := False;
  lblHost.Visible := False;
  lblTcpPort.Visible := False;

  if FConnectionChoice = accWiFi then
  begin
    btnClearWifi.Visible := True;
    lblStatusTitle.Caption := 'CONNEXION WI-FI';
    lblStatusText.Caption := 'Sélectionne le port COM puis clique sur CONNECTER.';
    lblUSBState.Caption := 'Config';
    lblWifiState.Caption := 'Attente';
  end
  else if FConnectionChoice = accBluetooth then
  begin
    lblStatusTitle.Caption := 'CONNEXION BLUETOOTH';
    lblStatusText.Caption :=
      'Sélectionne le port COM Bluetooth ATS-25X2 puis clique sur CONNECTER.';
    lblUSBState.Caption := 'BT prêt';
    lblWifiState.Caption := 'Inactif';
  end
  else
  begin
    lblStatusTitle.Caption := 'CONNEXION SERIE';
    lblStatusText.Caption := 'Sélectionne le port COM puis clique sur CONNECTER.';
    lblUSBState.Caption := 'Prêt';
    lblWifiState.Caption := 'Inactif';
  end;
end;


procedure TfrmSerialConnect.cbTransportChange(Sender: TObject);
begin
  case cbTransport.ItemIndex of
    1: FConnectionChoice := accWiFi;
    2: FConnectionChoice := accBluetooth;
  else
    FConnectionChoice := accSerial;
  end;

  UpdateTransportUI;
end;

procedure TfrmSerialConnect.btnClearWifiClick(Sender: TObject);
var
  Ini: TIniFile;
  IniFileName: string;
begin
  if MessageDlg(
       'Effacer le SSID, le mot de passe et l''ancienne adresse Wi-Fi mémorisés ?'#13#10#13#10 +
       'Ils devront être ressaisis lors de la prochaine configuration Wi-Fi.',
       mtConfirmation, [mbYes, mbNo], 0
     ) <> mrYes then
    Exit;

  IniFileName := ChangeFileExt(Application.ExeName, '.ini');
  Ini := TIniFile.Create(IniFileName);
  try
    Ini.DeleteKey('WiFi', 'SSID');
    Ini.DeleteKey('WiFi', 'Password');
    Ini.DeleteKey('WiFi', 'LastIP');
    Ini.UpdateFile;
  finally
    Ini.Free;
  end;

  edtSSID.Clear;
  edtWifiPass.Clear;
  edtHost.Text := 'ats25.local';

  FSSID := '';
  FWifiPassword := '';
  FSelectedHost := '';

  lblWifiState.Caption := 'A configurer';
  lblStatusTitle.Caption := 'PARAMETRES WI-FI EFFACES';
  lblStatusText.Caption :=
    'Clique sur CONNECTER pour saisir le nouveau SSID et le mot de passe.';
end;


procedure TfrmSerialConnect.btnWifiConfigClick(Sender: TObject);
begin
  if ConnectWiFiNow then
    ModalResult:=mrOk;
end;


procedure TfrmSerialConnect.imgRefreshClick(Sender: TObject);
begin
  LoadSerialPorts;
end;

procedure TfrmSerialConnect.imgConnectClick(Sender: TObject);
begin
  if FOperationBusy then
    Exit;

  FOperationBusy := True;
  try
      { En série le COM est obligatoire.
        En Wi-Fi, MainUnit essaie d'abord l'IP mémorisée sans USB. }
      if FConnectionChoice in [accSerial, accBluetooth] then
      begin
        if cbPort.ItemIndex < 0 then
        begin
          MessageDlg(
            'Sélectionne le port COM de l''ATS.',
            mtWarning, [mbOK], 0
          );
          Exit;
        end;

        FSelectedPort := cbPort.Text;
        FSelectedBaud := 115200;
      end
      else
      begin
        { Le COM reste mémorisé s'il est sélectionné :
          il servira uniquement comme secours si le Wi-Fi direct échoue. }
        if cbPort.ItemIndex >= 0 then
          FSelectedPort := cbPort.Text
        else
          FSelectedPort := '';

        FSelectedBaud := 115200;
      end;

      ModalResult := mrOk;
  finally
    FOperationBusy := False;
  end;
end;


procedure TfrmSerialConnect.imgCancelClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TfrmSerialConnect.imgCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

end.

