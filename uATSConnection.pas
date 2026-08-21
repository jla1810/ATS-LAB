unit uATSConnection;

interface

uses
  Winapi.Windows,
  Winapi.WinSock,
  System.SysUtils,
  System.Math,
  Vcl.Forms;

type
  TATSLogEvent = procedure(Sender: TObject; const AText: string) of object;

  TATSTransportType = (
    attNone,
    attSerial,
    attWiFi
  );

  TATSConnectionState = (
    acsDisconnected,
    acsConnecting,
    acsConnected,
    acsError
  );

  TATSConnection = class
  private
    FHandle: THandle;
    FSocket: TSocket;
    FWsaStarted: Boolean;
    FTransport: TATSTransportType;
    FPortName: string;
    FBaudRate: Integer;
    FHost: string;
    FTcpPort: Integer;
    FState: TATSConnectionState;
    FLastError: string;
    FOnLog: TATSLogEvent;
    FReceiveBuffer: AnsiString;

    procedure Log(const AText: string);
    procedure SetError(const AMessage: string);
    function ResolveHost(const AHost: string; out AAddr: u_long): Boolean;
    function ReadSerialChunk: Boolean;
    function ReadTCPChunk: Boolean;
  public
    constructor Create;
    destructor Destroy; override;

    function Connect(const APortName: string; ABaudRate: Integer): Boolean;
    function ConnectTCP(const AHost: string; ATcpPort: Integer): Boolean;
    procedure Disconnect;

    function IsAlive: Boolean;
    function SendText(const AText: AnsiString): Boolean;
    function ReadLine(out ALine: string; ATimeoutMs: Cardinal): Boolean;

    property Handle: THandle read FHandle;
    property Transport: TATSTransportType read FTransport;
    property State: TATSConnectionState read FState;
    property PortName: string read FPortName;
    property BaudRate: Integer read FBaudRate;
    property Host: string read FHost;
    property TcpPort: Integer read FTcpPort;
    property LastError: string read FLastError;
    property OnLog: TATSLogEvent read FOnLog write FOnLog;
  end;

implementation

const
  CMaxReceiveBuffer = 16384;


constructor TATSConnection.Create;
begin
  inherited Create;
  FHandle := INVALID_HANDLE_VALUE;
  FSocket := INVALID_SOCKET;
  FWsaStarted := False;
  FTransport := attNone;
  FState := acsDisconnected;
  FReceiveBuffer := '';
end;

destructor TATSConnection.Destroy;
begin
  Disconnect;
  inherited;
end;

procedure TATSConnection.Log(const AText: string);
begin
  if Assigned(FOnLog) then
    FOnLog(Self, AText);
end;

procedure TATSConnection.SetError(const AMessage: string);
begin
  FLastError := AMessage;
  FState := acsError;
  Log('ERREUR: ' + AMessage);
end;

function TATSConnection.ResolveHost(const AHost: string;
  out AAddr: u_long): Boolean;
var
  HostA: AnsiString;
  HostEnt: PHostEnt;
  DirectAddr: u_long;
begin
  Result := False;
  AAddr := 0;

  HostA := AnsiString(Trim(AHost));
  if HostA = '' then
    Exit;

  DirectAddr := inet_addr(PAnsiChar(HostA));

  if DirectAddr <> INADDR_NONE then
  begin
    AAddr := DirectAddr;
    Result := True;
    Exit;
  end;

  HostEnt := gethostbyname(PAnsiChar(HostA));
  if HostEnt = nil then
    Exit;

  if (HostEnt^.h_addr_list = nil) or
     (HostEnt^.h_addr_list^ = nil) then
    Exit;

  Move(HostEnt^.h_addr_list^^, AAddr, SizeOf(AAddr));
  Result := True;
end;


function TATSConnection.Connect(const APortName: string;
  ABaudRate: Integer): Boolean;
var
  DeviceName: string;
  DCB: TDCB;
  Timeouts: TCommTimeouts;
begin
  Result := False;
  Disconnect;

  FReceiveBuffer := '';
  FTransport := attSerial;
  FState := acsConnecting;
  FLastError := '';
  FPortName := APortName;
  FBaudRate := ABaudRate;

  Log('OUVERTURE SERIE ' + APortName + ' @ ' + IntToStr(ABaudRate));

  DeviceName := '\\.\' + APortName;

  FHandle := CreateFile(
    PChar(DeviceName),
    GENERIC_READ or GENERIC_WRITE,
    0,
    nil,
    OPEN_EXISTING,
    0,
    0
  );

  if FHandle = INVALID_HANDLE_VALUE then
  begin
    SetError(SysErrorMessage(GetLastError));
    Exit;
  end;

  FillChar(DCB, SizeOf(DCB), 0);
  DCB.DCBlength := SizeOf(DCB);

  if not GetCommState(FHandle, DCB) then
  begin
    SetError(SysErrorMessage(GetLastError));
    Disconnect;
    Exit;
  end;

  DCB.BaudRate := ABaudRate;
  DCB.ByteSize := 8;
  DCB.Parity := NOPARITY;
  DCB.StopBits := ONESTOPBIT;

  { Delphi 11 expose les options de controle de flux dans DCB.Flags. }
  DCB.Flags := DCB.Flags and not (
    $00000002 or  { fParity }
    $00000004 or  { fOutxCtsFlow }
    $00000008 or  { fOutxDsrFlow }
    $00000030 or  { fDtrControl }
    $00000040 or  { fDsrSensitivity }
    $00000100 or  { fOutX }
    $00000200 or  { fInX }
    $00000400 or  { fErrorChar }
    $00000800 or  { fNull }
    $00003000 or  { fRtsControl }
    $00004000     { fAbortOnError }
  );
  DCB.Flags := DCB.Flags or $00000001; { fBinary }


  if not SetCommState(FHandle, DCB) then
  begin
    SetError(SysErrorMessage(GetLastError));
    Disconnect;
    Exit;
  end;

  FillChar(Timeouts, SizeOf(Timeouts), 0);
  Timeouts.ReadIntervalTimeout := 20;
  Timeouts.ReadTotalTimeoutConstant := 50;
  Timeouts.WriteTotalTimeoutConstant := 500;

  if not SetCommTimeouts(FHandle, Timeouts) then
  begin
    SetError(SysErrorMessage(GetLastError));
    Disconnect;
    Exit;
  end;

  PurgeComm(
    FHandle,
    PURGE_RXABORT or PURGE_RXCLEAR or
    PURGE_TXABORT or PURGE_TXCLEAR
  );

  FState := acsConnected;
  Log('CONNECTE SERIE ' + APortName);
  Result := True;
end;

function TATSConnection.ConnectTCP(const AHost: string;
  ATcpPort: Integer): Boolean;
const
  CConnectTimeoutMs = 5000;
var
  WSAData: TWSAData;
  Addr: TSockAddrIn;
  IPAddr: u_long;
  NonBlocking: u_long;
  PortWord: Word;
  ConnectResult: Integer;
  E: Integer;
  WriteSet: TFDSet;
  ErrorSet: TFDSet;
  Timeout: TTimeVal;
  SelectResult: Integer;
  SocketError: Integer;
  SocketErrorLen: Integer;
begin
  Result := False;
  Disconnect;

  FReceiveBuffer := '';
  FTransport := attWiFi;
  FState := acsConnecting;
  FLastError := '';
  FHost := AHost;
  FTcpPort := ATcpPort;

  if (ATcpPort < 1) or (ATcpPort > 65535) then
  begin
    SetError('Port TCP hors plage : ' + IntToStr(ATcpPort));
    Exit;
  end;

  Log(Format('OUVERTURE WIFI %s:%d', [AHost, ATcpPort]));

  if WSAStartup($0202, WSAData) <> 0 then
  begin
    SetError('Impossible d''initialiser WinSock.');
    Exit;
  end;
  FWsaStarted := True;

  if not ResolveHost(AHost, IPAddr) then
  begin
    SetError('Adresse introuvable : ' + AHost);
    Disconnect;
    Exit;
  end;

  FSocket := socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if FSocket = INVALID_SOCKET then
  begin
    SetError('Création socket impossible : ' + IntToStr(WSAGetLastError));
    Disconnect;
    Exit;
  end;

  { Passer en non bloquant AVANT connect pour ne jamais immobiliser l'UI
    pendant le timeout TCP système. }
  NonBlocking := 1;
  if ioctlsocket(FSocket, FIONBIO, NonBlocking) <> 0 then
  begin
    SetError('Impossible de passer la socket en mode non bloquant : ' +
      IntToStr(WSAGetLastError));
    Disconnect;
    Exit;
  end;

  FillChar(Addr, SizeOf(Addr), 0);
  Addr.sin_family := AF_INET;
  PortWord := Word(ATcpPort);
  Addr.sin_port := htons(PortWord);
  Addr.sin_addr.S_addr := IPAddr;

  ConnectResult := Winapi.WinSock.connect(
    FSocket,
    TSockAddr(Addr),
    SizeOf(Addr)
  );

  if ConnectResult = SOCKET_ERROR then
  begin
    E := WSAGetLastError;
    if (E <> WSAEWOULDBLOCK) and (E <> WSAEINPROGRESS) then
    begin
      SetError('Connexion TCP impossible : ' + IntToStr(E));
      Disconnect;
      Exit;
    end;

    FD_ZERO(WriteSet);
    FD_ZERO(ErrorSet);
    FD_SET(FSocket, WriteSet);
    FD_SET(FSocket, ErrorSet);

    Timeout.tv_sec := CConnectTimeoutMs div 1000;
    Timeout.tv_usec := (CConnectTimeoutMs mod 1000) * 1000;

    SelectResult := select(0, nil, @WriteSet, @ErrorSet, @Timeout);
    if SelectResult = SOCKET_ERROR then
    begin
      SetError('Erreur attente connexion TCP : ' + IntToStr(WSAGetLastError));
      Disconnect;
      Exit;
    end;

    if SelectResult = 0 then
    begin
      SetError('Connexion TCP : délai dépassé.');
      Disconnect;
      Exit;
    end;

    SocketError := 0;
    SocketErrorLen := SizeOf(SocketError);
    if getsockopt(FSocket, SOL_SOCKET, SO_ERROR,
      PAnsiChar(@SocketError), SocketErrorLen) = SOCKET_ERROR then
    begin
      SetError('Impossible de vérifier la connexion TCP : ' +
        IntToStr(WSAGetLastError));
      Disconnect;
      Exit;
    end;

    if SocketError <> 0 then
    begin
      SetError('Connexion TCP impossible : ' + IntToStr(SocketError));
      Disconnect;
      Exit;
    end;
  end;

  FState := acsConnected;
  Log(Format('CONNECTE WIFI %s:%d', [AHost, ATcpPort]));
  Result := True;
end;


procedure TATSConnection.Disconnect;
begin
  if FHandle <> INVALID_HANDLE_VALUE then
  begin
    Log('DECONNEXION SERIE ' + FPortName);
    CloseHandle(FHandle);
    FHandle := INVALID_HANDLE_VALUE;
  end;

  if FSocket <> INVALID_SOCKET then
  begin
    Log(Format('DECONNEXION WIFI %s:%d', [FHost, FTcpPort]));
    shutdown(FSocket, SD_BOTH);
    closesocket(FSocket);
    FSocket := INVALID_SOCKET;
  end;

  if FWsaStarted then
  begin
    WSACleanup;
    FWsaStarted := False;
  end;

  FReceiveBuffer := '';
  FTransport := attNone;
  FState := acsDisconnected;
end;

function TATSConnection.IsAlive: Boolean;
var
  Errors: DWORD;
  Stat: TComStat;
  C: AnsiChar;
  R: Integer;
  E: Integer;
begin
  Result := False;

  if FState <> acsConnected then
    Exit;

  case FTransport of
    attSerial:
      begin
        if FHandle = INVALID_HANDLE_VALUE then
          Exit;

        FillChar(Stat, SizeOf(Stat), 0);
        Errors := 0;

        if not ClearCommError(FHandle, Errors, @Stat) then
        begin
          SetError(SysErrorMessage(GetLastError));
          Exit;
        end;

        Result := True;
      end;

    attWiFi:
      begin
        if FSocket = INVALID_SOCKET then
          Exit;

        R := recv(FSocket, C, 1, MSG_PEEK);

        if R = 0 then
        begin
          SetError('Connexion Wi-Fi fermée par l''ATS.');
          Exit;
        end;

        if R = SOCKET_ERROR then
        begin
          E := WSAGetLastError;
          if E = WSAEWOULDBLOCK then
            Result := True
          else
            SetError('Erreur socket : ' + IntToStr(E));
        end
        else
          Result := True;
      end;
  end;
end;

function TATSConnection.SendText(const AText: AnsiString): Boolean;
const
  CSendTimeoutMs = 2000;
var
  Written: DWORD;
  SentNow: Integer;
  TotalSent: Integer;
  E: Integer;
  StartTick: UInt64;
begin
  Result := False;

  if not IsAlive then
    Exit;

  Log('TX > ' + StringReplace(string(AText), #10, '', [rfReplaceAll]));

  case FTransport of
    attSerial:
      begin
        Written := 0;
        Result := WriteFile(
          FHandle,
          Pointer(AText)^,
          Length(AText),
          Written,
          nil
        ) and (Written = DWORD(Length(AText)));

        if not Result then
          SetError(SysErrorMessage(GetLastError));
      end;

    attWiFi:
      begin
        TotalSent := 0;
        StartTick := GetTickCount64;

        while TotalSent < Length(AText) do
        begin
          SentNow := Winapi.WinSock.send(
            FSocket,
            PAnsiChar(AText)[TotalSent],
            Length(AText) - TotalSent,
            0
          );

          if SentNow = SOCKET_ERROR then
          begin
            E := WSAGetLastError;
            if E = WSAEWOULDBLOCK then
            begin
              if GetTickCount64 - StartTick >= CSendTimeoutMs then
              begin
                SetError('Délai émission TCP dépassé.');
                Exit;
              end;
              Sleep(5);
              Continue;
            end;

            SetError('Erreur émission TCP : ' + IntToStr(E));
            Exit;
          end;

          if SentNow = 0 then
          begin
            SetError('Connexion TCP fermée pendant l''émission.');
            Exit;
          end;

          Inc(TotalSent, SentNow);
        end;

        Result := True;
      end;
  end;
end;

function TATSConnection.ReadSerialChunk: Boolean;
var
  Errors: DWORD;
  Stat: TComStat;
  Buffer: array[0..255] of AnsiChar;
  ReadCount: DWORD;
  Chunk: AnsiString;
  ToRead: DWORD;
begin
  Result := False;

  FillChar(Stat, SizeOf(Stat), 0);
  Errors := 0;

  if not ClearCommError(FHandle, Errors, @Stat) then
  begin
    SetError(SysErrorMessage(GetLastError));
    Exit;
  end;

  if Stat.cbInQue = 0 then
    Exit;

  ToRead := Stat.cbInQue;
  if ToRead > DWORD(SizeOf(Buffer)) then
    ToRead := DWORD(SizeOf(Buffer));

  ReadCount := 0;

  if not ReadFile(FHandle, Buffer, ToRead, ReadCount, nil) then
  begin
    SetError(SysErrorMessage(GetLastError));
    Exit;
  end;

  if ReadCount > 0 then
  begin
    SetString(Chunk, PAnsiChar(@Buffer[0]), ReadCount);
    FReceiveBuffer := FReceiveBuffer + Chunk;
    if Length(FReceiveBuffer) > CMaxReceiveBuffer then
      Delete(FReceiveBuffer, 1, Length(FReceiveBuffer) - CMaxReceiveBuffer);
    Result := True;
  end;
end;

function TATSConnection.ReadTCPChunk: Boolean;
var
  Buffer: array[0..255] of AnsiChar;
  R: Integer;
  E: Integer;
  Chunk: AnsiString;
begin
  Result := False;

  R := recv(FSocket, Buffer, SizeOf(Buffer), 0);

  if R = 0 then
  begin
    SetError('Connexion Wi-Fi fermée par l''ATS.');
    Exit;
  end;

  if R = SOCKET_ERROR then
  begin
    E := WSAGetLastError;
    if E <> WSAEWOULDBLOCK then
      SetError('Erreur réception TCP : ' + IntToStr(E));
    Exit;
  end;

  if R > 0 then
  begin
    SetString(Chunk, PAnsiChar(@Buffer[0]), R);
    FReceiveBuffer := FReceiveBuffer + Chunk;
    if Length(FReceiveBuffer) > CMaxReceiveBuffer then
      Delete(FReceiveBuffer, 1, Length(FReceiveBuffer) - CMaxReceiveBuffer);
    Result := True;
  end;
end;

function TATSConnection.ReadLine(out ALine: string;
  ATimeoutMs: Cardinal): Boolean;
var
  StartTick: UInt64;
  P: Integer;
begin
  Result := False;
  ALine := '';

  if not IsAlive then
    Exit;

  StartTick := GetTickCount64;

  repeat
    P := Pos(#10, string(FReceiveBuffer));

    if P > 0 then
    begin
      ALine := Trim(Copy(string(FReceiveBuffer), 1, P - 1));
      Delete(FReceiveBuffer, 1, P);

      if ALine <> '' then
      begin
        Log('RX < ' + ALine);
        Result := True;
        Exit;
      end;
    end;

    case FTransport of
      attSerial: ReadSerialChunk;
      attWiFi: ReadTCPChunk;
    end;

    if FState <> acsConnected then
      Exit;

    Sleep(5);
  until (GetTickCount64 - StartTick) >= ATimeoutMs;
end;

end.
