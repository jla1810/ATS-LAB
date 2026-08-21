unit uSteamVisuals;

interface

uses
  System.SysUtils, System.Classes, System.IOUtils, System.Math,
  Vcl.Graphics, Vcl.ExtCtrls,
  uGraphicsCache;

type
  TSteamFrameSet = class
  private
    FFrames: array of TPicture;
    FCount: Integer;
  public
    constructor Create(const AFolder, APattern: string; ACount: Integer);
    destructor Destroy; override;
    procedure AssignTo(ATarget: TImage; AIndex: Integer);
    property Count: Integer read FCount;
  end;

  TSteamLamp = class
  private
    FOffPicture: TPicture;
    FOnPicture: TPicture;
    FTarget: TImage;
    FIsOn: Boolean;
  public
    constructor Create(ATarget: TImage; const AOffFile, AOnFile: string);
    destructor Destroy; override;
    procedure SetState(AOn: Boolean);
    property IsOn: Boolean read FIsOn write SetState;
  end;

implementation

constructor TSteamFrameSet.Create(const AFolder, APattern: string;
  ACount: Integer);
var
  I: Integer;
  FileName: string;
begin
  inherited Create;
  FCount := Max(0, ACount);
  SetLength(FFrames, FCount);

  for I := 0 to FCount - 1 do
  begin
    FFrames[I] := TPicture.Create;
    FileName := TPath.Combine(AFolder, Format(APattern, [I]));
    if not FileExists(FileName) then
      raise Exception.CreateFmt('Asset graphique introuvable : %s', [FileName]);
    TGraphicsCache.AssignTo(FFrames[I], FileName);
  end;
end;

destructor TSteamFrameSet.Destroy;
var
  I: Integer;
begin
  for I := 0 to FCount - 1 do
    FFrames[I].Free;
  inherited;
end;

procedure TSteamFrameSet.AssignTo(ATarget: TImage; AIndex: Integer);
begin
  if (ATarget = nil) or (FCount = 0) then
    Exit;

  AIndex := EnsureRange(AIndex, 0, FCount - 1);
  ATarget.Picture.Assign(FFrames[AIndex]);
end;

constructor TSteamLamp.Create(ATarget: TImage; const AOffFile,
  AOnFile: string);
begin
  inherited Create;
  FTarget := ATarget;
  FOffPicture := TPicture.Create;
  FOnPicture := TPicture.Create;

  if not FileExists(AOffFile) then
    raise Exception.CreateFmt('Voyant OFF introuvable : %s', [AOffFile]);
  if not FileExists(AOnFile) then
    raise Exception.CreateFmt('Voyant ON introuvable : %s', [AOnFile]);

  TGraphicsCache.AssignTo(FOffPicture, AOffFile);
  TGraphicsCache.AssignTo(FOnPicture, AOnFile);
  SetState(False);
end;

destructor TSteamLamp.Destroy;
begin
  FOffPicture.Free;
  FOnPicture.Free;
  inherited;
end;

procedure TSteamLamp.SetState(AOn: Boolean);
begin
  FIsOn := AOn;
  if FTarget = nil then
    Exit;

  if FIsOn then
    FTarget.Picture.Assign(FOnPicture)
  else
    FTarget.Picture.Assign(FOffPicture);
end;

end.
