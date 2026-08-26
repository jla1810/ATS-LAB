unit ConnectionDiagnosticUnit;

interface

uses
  Winapi.Windows,
  System.SysUtils, System.Classes,
  Vcl.Forms, Vcl.StdCtrls, Vcl.ExtCtrls, Vcl.Controls, Vcl.Graphics,
  Vcl.Clipbrd;

type
  TConnectionDiagnosticData = record
    ConnectionState: string;
    Transport: string;
    Endpoint: string;
    ConnectedDuration: string;
    ReceiverModel: string;
    Firmware: string;
    Hardware: string;
    Chip: string;
    Frequency: string;
    Mode: string;
    CommandsSent: Integer;
    ResponsesReceived: Integer;
    PingsSent: Integer;
    PongsReceived: Integer;
    MissingPongs: Integer;
    LastLatency: string;
    LastCommand: string;
    LastCommandTime: string;
    LastResponse: string;
    LastResponseTime: string;
    LastError: string;
  end;

  TfrmConnectionDiagnostic = class(TForm)
  private
    FMemo: TMemo;
    procedure CopyClick(Sender: TObject);
    procedure CloseClick(Sender: TObject);
    procedure BuildInterface;
    function BuildReport(const AData: TConnectionDiagnosticData): string;
  public
    class procedure Execute(AOwner: TComponent;
      const AData: TConnectionDiagnosticData); static;
  end;

implementation

procedure TfrmConnectionDiagnostic.BuildInterface;
var
  BottomPanel: TPanel;
  CopyButton: TButton;
  CloseButton: TButton;
begin
  Caption := 'ATS LAB - Diagnostic de connexion';
  BorderStyle := bsDialog;
  BorderIcons := [biSystemMenu];
  ClientWidth := 650;
  ClientHeight := 545;
  Position := poOwnerFormCenter;
  Color := RGB(35, 27, 18);
  Font.Name := 'Segoe UI';
  Font.Size := 10;

  BottomPanel := TPanel.Create(Self);
  BottomPanel.Parent := Self;
  BottomPanel.Align := alBottom;
  BottomPanel.Height := 52;
  BottomPanel.BevelOuter := bvNone;
  BottomPanel.Color := Color;

  CopyButton := TButton.Create(Self);
  CopyButton.Parent := BottomPanel;
  CopyButton.Caption := 'COPIER LE RAPPORT';
  CopyButton.SetBounds(350, 10, 145, 32);
  CopyButton.OnClick := CopyClick;

  CloseButton := TButton.Create(Self);
  CloseButton.Parent := BottomPanel;
  CloseButton.Caption := 'FERMER';
  CloseButton.SetBounds(505, 10, 125, 32);
  CloseButton.Default := True;
  CloseButton.Cancel := True;
  CloseButton.ModalResult := mrClose;
  CloseButton.OnClick := CloseClick;

  FMemo := TMemo.Create(Self);
  FMemo.Parent := Self;
  FMemo.Align := alClient;
  FMemo.ReadOnly := True;
  FMemo.ScrollBars := ssVertical;
  FMemo.WordWrap := False;
  FMemo.BorderStyle := bsSingle;
  FMemo.Color := RGB(18, 15, 11);
  FMemo.Font.Name := 'Consolas';
  FMemo.Font.Size := 10;
  FMemo.Font.Color := RGB(239, 190, 87);
end;

function TfrmConnectionDiagnostic.BuildReport(
  const AData: TConnectionDiagnosticData): string;
var
  Lines: TStringList;
begin
  Lines := TStringList.Create;
  try
    Lines.Add('ATS LAB - DIAGNOSTIC DE CONNEXION');
    Lines.Add('Généré le : ' + FormatDateTime('dd/mm/yyyy hh:nn:ss', Now));
    Lines.Add('');
    Lines.Add('[CONNEXION]');
    Lines.Add('État              : ' + AData.ConnectionState);
    Lines.Add('Transport         : ' + AData.Transport);
    Lines.Add('Point de connexion: ' + AData.Endpoint);
    Lines.Add('Durée connectée   : ' + AData.ConnectedDuration);
    Lines.Add('Dernière erreur   : ' + AData.LastError);
    Lines.Add('');
    Lines.Add('[RÉCEPTEUR]');
    Lines.Add('Modèle            : ' + AData.ReceiverModel);
    Lines.Add('Firmware          : ' + AData.Firmware);
    Lines.Add('Matériel          : ' + AData.Hardware);
    Lines.Add('Circuit           : ' + AData.Chip);
    Lines.Add('Fréquence         : ' + AData.Frequency);
    Lines.Add('Mode              : ' + AData.Mode);
    Lines.Add('');
    Lines.Add('[COMMUNICATION]');
    Lines.Add(Format('Commandes TX      : %d', [AData.CommandsSent]));
    Lines.Add(Format('Réponses RX       : %d', [AData.ResponsesReceived]));
    Lines.Add(Format('PING envoyés      : %d', [AData.PingsSent]));
    Lines.Add(Format('PONG reçus        : %d', [AData.PongsReceived]));
    Lines.Add(Format('PONG manquants    : %d', [AData.MissingPongs]));
    Lines.Add('Dernière latence  : ' + AData.LastLatency);
    Lines.Add('');
    Lines.Add('[DERNIERS ÉCHANGES]');
    Lines.Add('TX ' + AData.LastCommandTime + ' : ' + AData.LastCommand);
    Lines.Add('RX ' + AData.LastResponseTime + ' : ' + AData.LastResponse);
    Result := Lines.Text;
  finally
    Lines.Free;
  end;
end;

procedure TfrmConnectionDiagnostic.CopyClick(Sender: TObject);
begin
  Clipboard.AsText := FMemo.Text;
end;

procedure TfrmConnectionDiagnostic.CloseClick(Sender: TObject);
begin
  ModalResult := mrClose;
end;

class procedure TfrmConnectionDiagnostic.Execute(AOwner: TComponent;
  const AData: TConnectionDiagnosticData);
var
  Form: TfrmConnectionDiagnostic;
begin
  Form := TfrmConnectionDiagnostic.CreateNew(AOwner);
  try
    Form.BuildInterface;
    Form.FMemo.Text := Form.BuildReport(AData);
    Form.ShowModal;
  finally
    Form.Free;
  end;
end;

end.
