unit FavoritesUnit;

interface

uses
  System.SysUtils, System.Classes, System.IniFiles, System.Math,
  Vcl.Controls, Vcl.Forms, Vcl.StdCtrls, Vcl.Dialogs;

type
  TFavoriteData = record
    Name: string;
    Band: string;
    FrequencyHz: Int64;
    Mode: string;
    Volume: Integer;
    Squelch: Integer;
    BFO: Integer;
  end;

  TfrmFavorites = class(TForm)
    lblTitle: TLabel;
    lblHint: TLabel;
    lstFavorites: TListBox;
    btnAdd: TButton;
    btnRename: TButton;
    btnUpdate: TButton;
    btnDelete: TButton;
    btnRecall: TButton;
    btnClose: TButton;
    procedure btnAddClick(Sender: TObject);
    procedure btnRenameClick(Sender: TObject);
    procedure btnUpdateClick(Sender: TObject);
    procedure btnDeleteClick(Sender: TObject);
    procedure btnRecallClick(Sender: TObject);
    procedure btnCloseClick(Sender: TObject);
    procedure lstFavoritesDblClick(Sender: TObject);
    procedure lstFavoritesClick(Sender: TObject);
  private
    FFavorites: TArray<TFavoriteData>;
    FCurrent: TFavoriteData;
    FSelected: TFavoriteData;
    FIniFileName: string;
    procedure LoadFavorites;
    procedure SaveFavorites;
    procedure RefreshList;
    procedure UpdateButtons;
    function SelectedIndex: Integer;
    function DefaultFavoriteName: string;
  public
    class function Execute(const AIniFileName: string;
      const ACurrent: TFavoriteData; out ASelected: TFavoriteData): Boolean;
  end;

implementation

{$R *.dfm}

const
  CMaxFavorites = 200;

class function TfrmFavorites.Execute(const AIniFileName: string;
  const ACurrent: TFavoriteData; out ASelected: TFavoriteData): Boolean;
var
  Form: TfrmFavorites;
begin
  Result := False;
  ASelected := Default(TFavoriteData);
  Form := TfrmFavorites.Create(nil);
  try
    Form.FIniFileName := AIniFileName;
    Form.FCurrent := ACurrent;
    Form.LoadFavorites;
    Form.RefreshList;
    if Form.ShowModal = mrOk then
    begin
      ASelected := Form.FSelected;
      Result := True;
    end;
  finally
    Form.Free;
  end;
end;

procedure TfrmFavorites.LoadFavorites;
var
  Ini: TIniFile;
  I, Count: Integer;
  Prefix: string;
begin
  SetLength(FFavorites, 0);
  Ini := TIniFile.Create(FIniFileName);
  try
    Count := EnsureRange(Ini.ReadInteger('Favorites', 'Count', 0),
      0, CMaxFavorites);
    SetLength(FFavorites, Count);
    for I := 0 to Count - 1 do
    begin
      Prefix := IntToStr(I) + '.';
      FFavorites[I].Name := Ini.ReadString('Favorites', Prefix + 'Name',
        'Favori ' + IntToStr(I + 1));
      FFavorites[I].Band := Ini.ReadString('Favorites', Prefix + 'Band', '');
      FFavorites[I].FrequencyHz := StrToInt64Def(
        Ini.ReadString('Favorites', Prefix + 'FrequencyHz', '0'), 0);
      FFavorites[I].Mode := UpperCase(
        Ini.ReadString('Favorites', Prefix + 'Mode', 'AM'));
      FFavorites[I].Volume := EnsureRange(
        Ini.ReadInteger('Favorites', Prefix + 'Volume', 50), 0, 100);
      FFavorites[I].Squelch := EnsureRange(
        Ini.ReadInteger('Favorites', Prefix + 'Squelch', 0), 0, 100);
      FFavorites[I].BFO := EnsureRange(
        Ini.ReadInteger('Favorites', Prefix + 'BFO', 0), -3000, 3000);
    end;
  finally
    Ini.Free;
  end;
end;

procedure TfrmFavorites.SaveFavorites;
var
  Ini: TIniFile;
  I: Integer;
  Prefix: string;
begin
  Ini := TIniFile.Create(FIniFileName);
  try
    Ini.EraseSection('Favorites');
    Ini.WriteInteger('Favorites', 'Count', Length(FFavorites));
    for I := 0 to High(FFavorites) do
    begin
      Prefix := IntToStr(I) + '.';
      Ini.WriteString('Favorites', Prefix + 'Name', FFavorites[I].Name);
      Ini.WriteString('Favorites', Prefix + 'Band', FFavorites[I].Band);
      Ini.WriteString('Favorites', Prefix + 'FrequencyHz',
        IntToStr(FFavorites[I].FrequencyHz));
      Ini.WriteString('Favorites', Prefix + 'Mode', FFavorites[I].Mode);
      Ini.WriteInteger('Favorites', Prefix + 'Volume', FFavorites[I].Volume);
      Ini.WriteInteger('Favorites', Prefix + 'Squelch', FFavorites[I].Squelch);
      Ini.WriteInteger('Favorites', Prefix + 'BFO', FFavorites[I].BFO);
    end;
    Ini.UpdateFile;
  finally
    Ini.Free;
  end;
end;

procedure TfrmFavorites.RefreshList;
var
  I: Integer;
begin
  lstFavorites.Items.BeginUpdate;
  try
    lstFavorites.Clear;
    for I := 0 to High(FFavorites) do
      lstFavorites.Items.Add(Format('%s   |   %.3f MHz   |   %s   |   %s',
        [FFavorites[I].Name, FFavorites[I].FrequencyHz / 1000000,
         FFavorites[I].Mode, FFavorites[I].Band]));
  finally
    lstFavorites.Items.EndUpdate;
  end;
  if lstFavorites.Count > 0 then
    lstFavorites.ItemIndex := 0;
  UpdateButtons;
end;

procedure TfrmFavorites.UpdateButtons;
var
  HasSelection: Boolean;
begin
  HasSelection := SelectedIndex >= 0;
  btnRename.Enabled := HasSelection;
  btnUpdate.Enabled := HasSelection;
  btnDelete.Enabled := HasSelection;
  btnRecall.Enabled := HasSelection;
end;

function TfrmFavorites.SelectedIndex: Integer;
begin
  Result := lstFavorites.ItemIndex;
  if (Result < 0) or (Result >= Length(FFavorites)) then
    Result := -1;
end;

function TfrmFavorites.DefaultFavoriteName: string;
begin
  Result := Format('%s %.3f MHz',
    [FCurrent.Mode, FCurrent.FrequencyHz / 1000000]);
end;

procedure TfrmFavorites.btnAddClick(Sender: TObject);
var
  Name: string;
  NewIndex: Integer;
begin
  if Length(FFavorites) >= CMaxFavorites then
  begin
    MessageDlg('La limite de 200 favoris est atteinte.',
      mtWarning, [mbOK], 0);
    Exit;
  end;

  Name := DefaultFavoriteName;
  if not InputQuery('AJOUTER UN FAVORI', 'Nom du favori :', Name) then
    Exit;
  Name := Trim(Name);
  if Name = '' then
  begin
    MessageDlg('Le nom du favori ne peut pas etre vide.',
      mtWarning, [mbOK], 0);
    Exit;
  end;

  NewIndex := Length(FFavorites);
  SetLength(FFavorites, NewIndex + 1);
  FFavorites[NewIndex] := FCurrent;
  FFavorites[NewIndex].Name := Name;
  SaveFavorites;
  RefreshList;
  lstFavorites.ItemIndex := NewIndex;
  UpdateButtons;
end;

procedure TfrmFavorites.btnRenameClick(Sender: TObject);
var
  I: Integer;
  Name: string;
begin
  I := SelectedIndex;
  if I < 0 then
    Exit;
  Name := FFavorites[I].Name;
  if not InputQuery('RENOMMER LE FAVORI', 'Nouveau nom :', Name) then
    Exit;
  Name := Trim(Name);
  if Name = '' then
    Exit;
  FFavorites[I].Name := Name;
  SaveFavorites;
  RefreshList;
  lstFavorites.ItemIndex := I;
  UpdateButtons;
end;

procedure TfrmFavorites.btnUpdateClick(Sender: TObject);
var
  I: Integer;
  Name: string;
begin
  I := SelectedIndex;
  if I < 0 then
    Exit;
  if MessageDlg('Remplacer les reglages du favori "' +
    FFavorites[I].Name + '" par l''etat radio courant ?',
    mtConfirmation, [mbYes, mbNo], 0) <> mrYes then
    Exit;

  Name := FFavorites[I].Name;
  FFavorites[I] := FCurrent;
  FFavorites[I].Name := Name;
  SaveFavorites;
  RefreshList;
  lstFavorites.ItemIndex := I;
  UpdateButtons;
end;

procedure TfrmFavorites.btnDeleteClick(Sender: TObject);
var
  I, J: Integer;
begin
  I := SelectedIndex;
  if I < 0 then
    Exit;
  if MessageDlg('Supprimer le favori "' + FFavorites[I].Name + '" ?',
    mtConfirmation, [mbYes, mbNo], 0) <> mrYes then
    Exit;

  for J := I to High(FFavorites) - 1 do
    FFavorites[J] := FFavorites[J + 1];
  SetLength(FFavorites, Length(FFavorites) - 1);
  SaveFavorites;
  RefreshList;
  if lstFavorites.Count > 0 then
    lstFavorites.ItemIndex := Min(I, lstFavorites.Count - 1);
  UpdateButtons;
end;

procedure TfrmFavorites.btnRecallClick(Sender: TObject);
var
  I: Integer;
begin
  I := SelectedIndex;
  if I < 0 then
    Exit;
  FSelected := FFavorites[I];
  ModalResult := mrOk;
end;

procedure TfrmFavorites.btnCloseClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TfrmFavorites.lstFavoritesDblClick(Sender: TObject);
begin
  btnRecallClick(Sender);
end;

procedure TfrmFavorites.lstFavoritesClick(Sender: TObject);
begin
  UpdateButtons;
end;

end.
