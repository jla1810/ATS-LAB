unit uGraphicsCache;

interface

uses
  System.SysUtils, System.Classes, System.IOUtils,
  System.Generics.Collections,
  Vcl.Graphics, Vcl.Imaging.pngimage;

type
  TGraphicsCache = class
  private
    class var FPictures: TObjectDictionary<string, TPicture>;
    class function NormalizeFileName(const AFileName: string): string; static;
  public
    class procedure Initialize(const ARootFolder: string); static;
    class procedure Finalize; static;
    class function IsInitialized: Boolean; static;
    class function Contains(const AFileName: string): Boolean; static;
    class procedure AssignTo(APicture: TPicture;
      const AFileName: string); static;
    class function Count: Integer; static;
  end;

implementation

class function TGraphicsCache.NormalizeFileName(
  const AFileName: string): string;
begin
  Result := LowerCase(TPath.GetFullPath(AFileName));
end;

class procedure TGraphicsCache.Initialize(const ARootFolder: string);
var
  FileName: string;
  Key: string;
  Picture: TPicture;
begin
  Finalize;

  if not TDirectory.Exists(ARootFolder) then
    raise Exception.CreateFmt(
      'Dossier graphique introuvable : %s',
      [ARootFolder]
    );

  FPictures := TObjectDictionary<string, TPicture>.Create([doOwnsValues]);

  for FileName in TDirectory.GetFiles(
    ARootFolder,
    '*.png',
    TSearchOption.soAllDirectories
  ) do
  begin
    Key := NormalizeFileName(FileName);

    if FPictures.ContainsKey(Key) then
      Continue;

    Picture := TPicture.Create;
    try
      Picture.LoadFromFile(FileName);
      FPictures.Add(Key, Picture);
    except
      Picture.Free;
      raise;
    end;
  end;
end;

class procedure TGraphicsCache.Finalize;
begin
  FreeAndNil(FPictures);
end;

class function TGraphicsCache.IsInitialized: Boolean;
begin
  Result := FPictures <> nil;
end;

class function TGraphicsCache.Contains(const AFileName: string): Boolean;
begin
  Result :=
    (FPictures <> nil) and
    FPictures.ContainsKey(NormalizeFileName(AFileName));
end;

class procedure TGraphicsCache.AssignTo(APicture: TPicture;
  const AFileName: string);
var
  CachedPicture: TPicture;
  Key: string;
begin
  if APicture = nil then
    raise Exception.Create('Image cible non définie');

  if FPictures = nil then
    raise Exception.Create('Le cache graphique n''est pas initialisé');

  Key := NormalizeFileName(AFileName);

  if not FPictures.TryGetValue(Key, CachedPicture) then
    raise Exception.CreateFmt(
      'Image absente du cache : %s',
      [AFileName]
    );

  APicture.Assign(CachedPicture);
end;

class function TGraphicsCache.Count: Integer;
begin
  if FPictures = nil then
    Result := 0
  else
    Result := FPictures.Count;
end;

end.
