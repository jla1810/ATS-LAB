unit uSteamButton;

interface

uses
  System.SysUtils, System.Classes,
  Vcl.Controls, Vcl.ExtCtrls, Vcl.Graphics,
  uGraphicsCache;

type
  TSteamButton = class
  private
    FTarget: TImage;
    FOffPicture: TPicture;
    FOnPicture: TPicture;
    FPressedPicture: TPicture;
    FDown: Boolean;
    FPressed: Boolean;
    FOriginalTop: Integer;
    FOldMouseDown: TMouseEvent;
    FOldMouseUp: TMouseEvent;
    procedure TargetMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure TargetMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Refresh;
  public
    constructor Create(ATarget: TImage; const AOffFile, AOnFile,
      APressedFile: string);
    destructor Destroy; override;
    procedure SetDown(AValue: Boolean);
    property Down: Boolean read FDown write SetDown;
  end;

implementation

type
  TControlAccess = class(TControl);

constructor TSteamButton.Create(ATarget: TImage; const AOffFile, AOnFile,
  APressedFile: string);
begin
  inherited Create;

  if ATarget = nil then
    raise Exception.Create('Image du bouton non définie');

  FTarget := ATarget;
  FOriginalTop := FTarget.Top;

  FOffPicture := TPicture.Create;
  FOnPicture := TPicture.Create;
  FPressedPicture := TPicture.Create;

  if not FileExists(AOffFile) then
    raise Exception.CreateFmt('Image bouton OFF introuvable : %s', [AOffFile]);
  if not FileExists(AOnFile) then
    raise Exception.CreateFmt('Image bouton ON introuvable : %s', [AOnFile]);
  if not FileExists(APressedFile) then
    raise Exception.CreateFmt('Image bouton appuyé introuvable : %s',
      [APressedFile]);

  TGraphicsCache.AssignTo(FOffPicture, AOffFile);
  TGraphicsCache.AssignTo(FOnPicture, AOnFile);
  TGraphicsCache.AssignTo(FPressedPicture, APressedFile);

  FOldMouseDown := TControlAccess(FTarget).OnMouseDown;
  FOldMouseUp := TControlAccess(FTarget).OnMouseUp;
  TControlAccess(FTarget).OnMouseDown := TargetMouseDown;
  TControlAccess(FTarget).OnMouseUp := TargetMouseUp;

  Refresh;
end;

destructor TSteamButton.Destroy;
begin
  if FTarget <> nil then
  begin
    FTarget.Top := FOriginalTop;
    TControlAccess(FTarget).OnMouseDown := FOldMouseDown;
    TControlAccess(FTarget).OnMouseUp := FOldMouseUp;
  end;

  FOffPicture.Free;
  FOnPicture.Free;
  FPressedPicture.Free;
  inherited;
end;

procedure TSteamButton.SetDown(AValue: Boolean);
begin
  if FDown = AValue then
    Exit;

  FDown := AValue;
  Refresh;
end;

procedure TSteamButton.Refresh;
begin
  if FTarget = nil then
    Exit;

  if FPressed then
    FTarget.Picture.Assign(FPressedPicture)
  else if FDown then
    FTarget.Picture.Assign(FOnPicture)
  else
    FTarget.Picture.Assign(FOffPicture);
end;

procedure TSteamButton.TargetMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button = mbLeft then
  begin
    FPressed := True;
    FTarget.Top := FOriginalTop + 1;
    Refresh;
  end;

  if Assigned(FOldMouseDown) then
    FOldMouseDown(Sender, Button, Shift, X, Y);
end;

procedure TSteamButton.TargetMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button = mbLeft then
  begin
    FPressed := False;
    FTarget.Top := FOriginalTop;
    Refresh;
  end;

  if Assigned(FOldMouseUp) then
    FOldMouseUp(Sender, Button, Shift, X, Y);
end;

end.
