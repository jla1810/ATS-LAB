unit uATSProtocol;

interface

uses
  System.SysUtils,
  System.Math;

type
  TATSStatus = record
    FrequencyKHz: Int64;
    Mode: string;
    Volume: Integer;
    Squelch: Integer;
    BFO: Integer;
    RSSI: Integer;
    SNR: Integer;
    Band: string;
    HasVolume: Boolean;
    HasSquelch: Boolean;
    HasBFO: Boolean;
    HasRSSI: Boolean;
    HasSNR: Boolean;
    HasBand: Boolean;
    Valid: Boolean;
  end;

  TATSMode = (
    atmAM,
    atmFM,
    atmLSB,
    atmUSB,
    atmCW,
    atmCWR
  );

  TATSProtocol = class
  public
    class function Ping: AnsiString; static;
    class function RequestStatus: AnsiString; static;
    class function SetFrequencyKHz(AFrequencyKHz: Int64): AnsiString; static;
    class function SetMode(AMode: TATSMode): AnsiString; static;
    class function SetRDS(AEnabled: Boolean): AnsiString; static;
    class function SetVolume(AValue: Integer): AnsiString; static;
    class function SetSquelch(AValue: Integer): AnsiString; static;
    class function SetBFO(AValueHz: Integer): AnsiString; static;
    class function ScanStart: AnsiString; static;
    class function ScanStartAt(AStartKHz: Int64;
      AStepKHz: Double): AnsiString; static;
    class function ScanStop: AnsiString; static;
    class function RequestScanData: AnsiString; static;
    class function IsPong(const AResponse: string): Boolean; static;
    class function ParseStatus(const AResponse: string;
      out AStatus: TATSStatus): Boolean; static;
  end;

implementation

class function TATSProtocol.Ping: AnsiString;
begin
  Result := 'PING'#10;
end;

class function TATSProtocol.RequestStatus: AnsiString;
begin
  Result := 'STATUS?'#10;
end;

class function TATSProtocol.SetFrequencyKHz(
  AFrequencyKHz: Int64): AnsiString;
begin
  Result := AnsiString(
    'FREQ=' + IntToStr(Max(Int64(0), AFrequencyKHz)) + #10
  );
end;

class function TATSProtocol.SetMode(AMode: TATSMode): AnsiString;
const
  CModeNames: array[TATSMode] of string = (
    'AM', 'FM', 'LSB', 'USB', 'CW', 'CWR'
  );
begin
  Result := AnsiString('MODE=' + CModeNames[AMode] + #10);
end;

class function TATSProtocol.SetVolume(AValue: Integer): AnsiString;
begin
  Result := AnsiString(
    'VOL=' + IntToStr(EnsureRange(AValue, 0, 100)) + #10
  );
end;

class function TATSProtocol.SetSquelch(AValue: Integer): AnsiString;
begin
  Result := AnsiString(
    'SQ=' + IntToStr(EnsureRange(AValue, 0, 100)) + #10
  );
end;

class function TATSProtocol.SetBFO(AValueHz: Integer): AnsiString;
begin
  Result := AnsiString(
    'BFO=' + IntToStr(EnsureRange(AValueHz, -3000, 3000)) + #10
  );
end;

class function TATSProtocol.ScanStart: AnsiString;
begin
  Result := 'SCAN=START'#10;
end;

class function TATSProtocol.ScanStartAt(AStartKHz: Int64;
  AStepKHz: Double): AnsiString;
var
  FS: TFormatSettings;
begin
  FS := TFormatSettings.Invariant;
  Result := AnsiString(
    'SCAN=START,BASE_KHZ=' + IntToStr(AStartKHz) +
    ',STEP_KHZ=' + Format('%.3f', [AStepKHz], FS) + #10
  );
end;

class function TATSProtocol.ScanStop: AnsiString;
begin
  Result := 'SCAN=STOP'#10;
end;

class function TATSProtocol.RequestScanData: AnsiString;
begin
  Result := 'SCANDATA?'#10;
end;

class function TATSProtocol.IsPong(const AResponse: string): Boolean;
var
  S: string;
begin
  S := UpperCase(Trim(AResponse));
  Result := (S = 'PONG') or (S = 'OK,PONG');
end;

class function TATSProtocol.ParseStatus(const AResponse: string;
  out AStatus: TATSStatus): Boolean;
var
  Parts: TArray<string>;
  Item: string;
  P: Integer;
  Key: string;
  Value: string;
  IntValue: Integer;
  Int64Value: Int64;
begin
  AStatus.FrequencyKHz := 0;
  AStatus.Mode := '';
  AStatus.Volume := 0;
  AStatus.Squelch := 0;
  AStatus.BFO := 0;
  AStatus.RSSI := 0;
  AStatus.SNR := 0;
  AStatus.Band := '';
  AStatus.HasVolume := False;
  AStatus.HasSquelch := False;
  AStatus.HasBFO := False;
  AStatus.HasRSSI := False;
  AStatus.HasSNR := False;
  AStatus.HasBand := False;
  AStatus.Valid := False;

  Parts := AResponse.Split([',']);

  for Item in Parts do
  begin
    P := Pos('=', Item);
    if P <= 0 then
      Continue;

    Key := UpperCase(Trim(Copy(Item, 1, P - 1)));
    Value := Trim(Copy(Item, P + 1, MaxInt));

    if (Key = 'FREQ_KHZ') or (Key = 'FREQ') then
    begin
      if TryStrToInt64(Value, Int64Value) and (Int64Value > 0) then
        AStatus.FrequencyKHz := Int64Value;
    end
    else if Key = 'MODE' then
    begin
      Value := UpperCase(Value);
      if (Value = 'AM') or (Value = 'FM') or (Value = 'USB') or
         (Value = 'LSB') or (Value = 'CW') or (Value = 'CWR') or
         (Value = 'CW-R') then
        AStatus.Mode := Value;
    end
    else if Key = 'VOL' then
    begin
      if TryStrToInt(Value, IntValue) then
      begin
        AStatus.Volume := IntValue;
        AStatus.HasVolume := True;
      end;
    end
    else if (Key = 'SQ') or (Key = 'SQL') or (Key = 'SQUELCH') then
    begin
      if TryStrToInt(Value, IntValue) then
      begin
        AStatus.Squelch := IntValue;
        AStatus.HasSquelch := True;
      end;
    end
    else if Key = 'BFO' then
    begin
      if TryStrToInt(Value, IntValue) then
      begin
        AStatus.BFO := IntValue;
        AStatus.HasBFO := True;
      end;
    end
    else if Key = 'RSSI' then
    begin
      if TryStrToInt(Value, IntValue) then
      begin
        AStatus.RSSI := IntValue;
        AStatus.HasRSSI := True;
      end;
    end
    else if Key = 'SNR' then
    begin
      if TryStrToInt(Value, IntValue) then
      begin
        AStatus.SNR := IntValue;
        AStatus.HasSNR := True;
      end;
    end
    else if Key = 'BAND' then
    begin
      AStatus.Band := UpperCase(Value);
      AStatus.HasBand := AStatus.Band <> '';
    end;
  end;

  AStatus.Valid :=
    (AStatus.FrequencyKHz > 0) and
    (AStatus.Mode <> '');

  Result := AStatus.Valid;
end;


class function TATSProtocol.SetRDS(AEnabled: Boolean): AnsiString;
begin
  if AEnabled then
    Result := AnsiString('RDS=ON'#10)
  else
    Result := AnsiString('RDS=OFF'#10);
end;

end.
