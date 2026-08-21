unit uRotatingNeedle;

interface

uses
  Winapi.GDIPAPI, Winapi.GDIPOBJ,
  System.SysUtils, System.Classes,
  Vcl.ExtCtrls;

type
  TRotatingNeedle = class
  private
    FPaintBox: TPaintBox;
    FImage: TGPImage;
    FAngle: Single;
    FPivotX: Single;
    FPivotY: Single;
    procedure PaintNeedle(Sender: TObject);
  public
    constructor Create(APaintBox: TPaintBox; const AFileName: string;
      APivotX, APivotY: Single);
    destructor Destroy; override;
    procedure SetAngle(const AAngle: Single);
    property Angle: Single read FAngle write SetAngle;
  end;

implementation

constructor TRotatingNeedle.Create(APaintBox: TPaintBox;
  const AFileName: string; APivotX, APivotY: Single);
begin
  inherited Create;
  if APaintBox = nil then
    raise Exception.Create('PaintBox du S-mètre non défini');
  if not FileExists(AFileName) then
    raise Exception.CreateFmt('Aiguille introuvable : %s', [AFileName]);

  FPaintBox := APaintBox;
  FPivotX := APivotX;
  FPivotY := APivotY;
  FImage := TGPImage.Create(AFileName);

  if FImage.GetLastStatus <> Ok then
    raise Exception.CreateFmt('Impossible de charger : %s', [AFileName]);

  FPaintBox.OnPaint := PaintNeedle;
end;

destructor TRotatingNeedle.Destroy;
begin
  if FPaintBox <> nil then
    FPaintBox.OnPaint := nil;
  FPaintBox := nil;
  FImage.Free;
  inherited;
end;

procedure TRotatingNeedle.SetAngle(const AAngle: Single);
begin
  FAngle := AAngle;
  if FPaintBox <> nil then
    FPaintBox.Invalidate;
end;

procedure TRotatingNeedle.PaintNeedle(Sender: TObject);
var
  G: TGPGraphics;
  State: GraphicsState;
begin
  if (FImage = nil) or (FPaintBox = nil) then Exit;

  G := TGPGraphics.Create(FPaintBox.Canvas.Handle);
  try
    G.SetSmoothingMode(SmoothingModeHighQuality);
    G.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    State := G.Save;
    try
      G.TranslateTransform(-FPivotX, -FPivotY, MatrixOrderAppend);
      G.RotateTransform(FAngle, MatrixOrderAppend);
      G.TranslateTransform(FPivotX, FPivotY, MatrixOrderAppend);
      G.DrawImage(FImage, 0, 0, FPaintBox.Width, FPaintBox.Height);
    finally
      G.Restore(State);
    end;
  finally
    G.Free;
  end;
end;

end.
