unit uSteamKnob;

interface

uses
    System.Types,
Winapi.Windows,
  System.SysUtils, System.Classes, System.Math,
  Vcl.Controls, Vcl.ExtCtrls, Vcl.Forms,
  uSteamVisuals;

type
  TSteamKnob = class
  private
    FTarget: TImage;
    FFrames: TSteamFrameSet;
    FValue: Integer;
    FMinValue: Integer;
    FMaxValue: Integer;
    FDefaultValue: Integer;
    FStep: Integer;
    FDragging: Boolean;
    FLastY: Integer;
    FOnChange: TNotifyEvent;
    procedure SetValue(const AValue: Integer);
    procedure RefreshFrame;
    procedure TargetMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure TargetMouseMove(Sender: TObject; Shift: TShiftState;
      X, Y: Integer);
    procedure TargetMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure TargetDblClick(Sender: TObject);
    procedure TargetMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
  public
    constructor Create(ATarget: TImage; const AFramesFolder: string;
      AMinValue, AMaxValue, ADefaultValue, AStep: Integer);
    destructor Destroy; override;
    property Value: Integer read FValue write SetValue;
    property MinValue: Integer read FMinValue;
    property MaxValue: Integer read FMaxValue;
    property DefaultValue: Integer read FDefaultValue;
    property OnChange: TNotifyEvent read FOnChange write FOnChange;
  end;

implementation

const
  CKnobAngleMin = -120.0;
  CKnobAngleMax = 120.0;
  CKnobTotalSweep = 240.0;


type
  TControlAccess = class(TControl);


constructor TSteamKnob.Create(ATarget: TImage; const AFramesFolder: string;
  AMinValue, AMaxValue, ADefaultValue, AStep: Integer);
begin
  inherited Create;

  if ATarget = nil then
    raise Exception.Create('Image du bouton rotatif non définie');

  FTarget := ATarget;
  FMinValue := AMinValue;
  FMaxValue := Max(AMinValue + 1, AMaxValue);
  FDefaultValue := EnsureRange(ADefaultValue, FMinValue, FMaxValue);
  FStep := Max(1, Abs(AStep));
  FValue := FDefaultValue;

  FFrames := TSteamFrameSet.Create(
    AFramesFolder,
    'smallknob_%.2d.png',
    48
  );

  FTarget.Cursor := crSizeNS;
  FTarget.OnMouseDown := TargetMouseDown;
  FTarget.OnMouseMove := TargetMouseMove;
  FTarget.OnMouseUp := TargetMouseUp;
  FTarget.OnDblClick := TargetDblClick;
  TControlAccess(FTarget).OnMouseWheel := TargetMouseWheel;

  RefreshFrame;
end;

destructor TSteamKnob.Destroy;
begin
  if FTarget <> nil then
  begin
    FTarget.OnMouseDown := nil;
    FTarget.OnMouseMove := nil;
    FTarget.OnMouseUp := nil;
    FTarget.OnDblClick := nil;
    TControlAccess(FTarget).OnMouseWheel := nil;
  end;

  FFrames.Free;
  inherited;
end;

procedure TSteamKnob.SetValue(const AValue: Integer);
var
  NewValue: Integer;
begin
  NewValue := EnsureRange(AValue, FMinValue, FMaxValue);
  if NewValue = FValue then
    Exit;

  FValue := NewValue;
  RefreshFrame;

  if Assigned(FOnChange) then
    FOnChange(Self);
end;

procedure TSteamKnob.RefreshFrame;
var
  FrameIndex: Integer;
begin
  if (FFrames = nil) or (FTarget = nil) then
    Exit;

  FrameIndex := Round(
    (FValue - FMinValue) * (FFrames.Count - 1) /
    (FMaxValue - FMinValue)
  );

  FFrames.AssignTo(FTarget, FrameIndex);
end;

procedure TSteamKnob.TargetMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  Form: TCustomForm;
begin
  if Button <> mbLeft then
    Exit;

  FDragging := True;
  FLastY := Y;

  Form := GetParentForm(FTarget);
  if Form <> nil then
    SetCapture(Form.Handle);
end;

procedure TSteamKnob.TargetMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var
  DeltaY: Integer;
begin
  if not FDragging then
    Exit;

  DeltaY := FLastY - Y;
  if Abs(DeltaY) >= 2 then
  begin
    SetValue(FValue + Sign(DeltaY) * FStep);
    FLastY := Y;
  end;
end;

procedure TSteamKnob.TargetMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button <> mbLeft then
    Exit;

  FDragging := False;
  ReleaseCapture;
end;

procedure TSteamKnob.TargetDblClick(Sender: TObject);
begin
  SetValue(FDefaultValue);
end;

procedure TSteamKnob.TargetMouseWheel(Sender: TObject; Shift: TShiftState;
  WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
begin
  SetValue(FValue + Sign(WheelDelta) * FStep);
  Handled := True;
end;

end.
