unit uImageCache;

interface

uses
  System.SysUtils, System.Classes, System.IOUtils,
  System.Generics.Collections, Vcl.Graphics;

type
  TImageCache = class
  strict private
    class var FItems: TObjectDictionary<string, TPicture>;
    class function KeyOf(const AFileName: string): string; static;
    class procedure AddFile(const AFileName: string); static;
  public
    class procedure Initialize(const ARootFolder: string); static;
    class procedure Finalize; static;
    class function Picture(const AFileName: string): TPicture; static;
    class function Count: Integer; static;
  end;

implementation

class function TImageCache.KeyOf(const AFileName: string): string;
begin
  Result := LowerCase(TPath.GetFullPath(AFileName));
end;

class procedure TImageCache.AddFile(const AFileName: string);
var
  P: TPicture;
  K: string;
begin
  if not FileExists(AFileName) then
    Exit;

  K := KeyOf(AFileName);
  if FItems.ContainsKey(K) then
    Exit;

  P := TPicture.Create;
  try
    P.LoadFromFile(AFileName);
    FItems.Add(K, P);
  except
    P.Free;
    raise;
  end;
end;

class procedure TImageCache.Initialize(const ARootFolder: string);
var
  FileName: string;
begin
  Finalize;
  FItems := TObjectDictionary<string, TPicture>.Create([doOwnsValues]);

  if not TDirectory.Exists(ARootFolder) then
    raise Exception.CreateFmt('Dossier Data introuvable : %s', [ARootFolder]);

  for FileName in TDirectory.GetFiles(
    ARootFolder, '*.png', TSearchOption.soAllDirectories
  ) do
    AddFile(FileName);
end;

class procedure TImageCache.Finalize;
begin
  FreeAndNil(FItems);
end;

class function TImageCache.Picture(const AFileName: string): TPicture;
var
  K: string;
begin
  if FItems = nil then
    raise Exception.Create('Le cache graphique n''est pas initialisé');

  K := KeyOf(AFileName);
  if not FItems.TryGetValue(K, Result) then
    raise Exception.CreateFmt('Image absente du cache : %s', [AFileName]);
end;

class function TImageCache.Count: Integer;
begin
  if FItems = nil then
    Result := 0
  else
    Result := FItems.Count;
end;

end.
