unit uSteamButtonManager;

interface

uses
  System.SysUtils, System.Classes, System.Generics.Collections,
  Vcl.ExtCtrls, uSteamButton;

type
  TSteamButtonKind = (sbkRadio, sbkToggle);

  TSteamButtonItem = class
  public
    Image: TImage;
    Visual: TSteamButton;
    GroupIndex: Integer;
    Kind: TSteamButtonKind;
    Command: string;
    State: Boolean;
    destructor Destroy; override;
  end;

  TSteamButtonCommandEvent = procedure(Sender: TObject;
    const ACommand: string; AState: Boolean) of object;

  TSteamButtonManager = class
  private
    FItems: TObjectList<TSteamButtonItem>;
    FOnCommand: TSteamButtonCommandEvent;
    procedure ImageClick(Sender: TObject);
    function FindByImage(AImage: TImage): TSteamButtonItem;
    procedure RefreshItem(AItem: TSteamButtonItem);
    procedure SelectRadio(ASelected: TSteamButtonItem);
  public
    constructor Create;
    destructor Destroy; override;
    function AddButton(AImage: TImage; AGroupIndex: Integer;
      AKind: TSteamButtonKind; const ACommand, AOffFile,
      AOnFile, APressedFile: string; AInitialState: Boolean = False):
      TSteamButtonItem;
    procedure SetState(const ACommand: string; AState: Boolean);
    function GetState(const ACommand: string): Boolean;
    property OnCommand: TSteamButtonCommandEvent read FOnCommand write FOnCommand;
  end;

implementation

destructor TSteamButtonItem.Destroy;
begin
  Visual.Free;
  inherited;
end;


constructor TSteamButtonManager.Create;
begin
  inherited Create;
  FItems := TObjectList<TSteamButtonItem>.Create(True);
end;

destructor TSteamButtonManager.Destroy;
var
  Item: TSteamButtonItem;
begin
  if FItems <> nil then
    for Item in FItems do
      if Item.Image <> nil then
        Item.Image.OnClick := nil;

  FItems.Free;
  inherited;
end;

function TSteamButtonManager.AddButton(AImage: TImage;
  AGroupIndex: Integer; AKind: TSteamButtonKind;
  const ACommand, AOffFile, AOnFile, APressedFile: string;
  AInitialState: Boolean): TSteamButtonItem;
begin
  if AImage = nil then
    raise Exception.Create('Image de bouton non définie');

  Result := TSteamButtonItem.Create;
  Result.Image := AImage;
  Result.GroupIndex := AGroupIndex;
  Result.Kind := AKind;
  Result.Command := ACommand;
  Result.State := AInitialState;
  Result.Visual := TSteamButton.Create(
    AImage, AOffFile, AOnFile, APressedFile
  );

  AImage.OnClick := ImageClick;
  FItems.Add(Result);
  RefreshItem(Result);
end;

function TSteamButtonManager.FindByImage(
  AImage: TImage): TSteamButtonItem;
var
  Item: TSteamButtonItem;
begin
  Result := nil;
  for Item in FItems do
    if Item.Image = AImage then
      Exit(Item);
end;

procedure TSteamButtonManager.RefreshItem(AItem: TSteamButtonItem);
begin
  if (AItem <> nil) and (AItem.Visual <> nil) then
    AItem.Visual.SetDown(AItem.State);
end;

procedure TSteamButtonManager.SelectRadio(ASelected: TSteamButtonItem);
var
  Item: TSteamButtonItem;
  NewState: Boolean;
begin
  for Item in FItems do
    if (Item.Kind = sbkRadio) and
       (Item.GroupIndex = ASelected.GroupIndex) then
    begin
      NewState := Item = ASelected;

      { Ne recharge l'image que si l'état change réellement.
        Cela supprime le scintillement causé par STATUS? répété. }
      if Item.State <> NewState then
      begin
        Item.State := NewState;
        RefreshItem(Item);
      end;
    end;
end;


procedure TSteamButtonManager.ImageClick(Sender: TObject);
var
  Item: TSteamButtonItem;
begin
  if not (Sender is TImage) then
    Exit;

  Item := FindByImage(TImage(Sender));
  if Item = nil then
    Exit;

  if Item.Kind = sbkRadio then
    SelectRadio(Item)
  else
  begin
    Item.State := not Item.State;
    RefreshItem(Item);
  end;

  if Assigned(FOnCommand) then
    FOnCommand(Self, Item.Command, Item.State);
end;

procedure TSteamButtonManager.SetState(
  const ACommand: string; AState: Boolean);
var
  Item: TSteamButtonItem;
begin
  for Item in FItems do
    if SameText(Item.Command, ACommand) then
    begin
      if (Item.Kind = sbkRadio) and AState then
      begin
        if not Item.State then
          SelectRadio(Item);
      end
      else if Item.State <> AState then
      begin
        Item.State := AState;
        RefreshItem(Item);
      end;
      Exit;
    end;
end;


function TSteamButtonManager.GetState(
  const ACommand: string): Boolean;
var
  Item: TSteamButtonItem;
begin
  Result := False;
  for Item in FItems do
    if SameText(Item.Command, ACommand) then
      Exit(Item.State);
end;

end.
