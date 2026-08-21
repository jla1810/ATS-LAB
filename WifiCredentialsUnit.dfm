object frmWifiCredentials: TfrmWifiCredentials
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Connexion Wi-Fi ATS'
  ClientHeight = 208
  ClientWidth = 470
  Color = 2105376
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clSilver
  Font.Height = -13
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnCreate = FormCreate
  TextHeight = 17
  object lblTitle: TLabel
    Left = 24
    Top = 18
    Width = 294
    Height = 19
    Caption = 'Paramètres du réseau Wi-Fi de l''ATS'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 8454143
    Font.Height = -16
    Font.Name = 'Segoe UI'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lblSSID: TLabel
    Left = 24
    Top = 64
    Width = 30
    Height = 17
    Caption = 'SSID'
  end
  object lblPassword: TLabel
    Left = 24
    Top = 108
    Width = 78
    Height = 17
    Caption = 'Mot de passe'
  end
  object edtSSID: TEdit
    Left = 128
    Top = 60
    Width = 310
    Height = 25
    TabOrder = 0
  end
  object edtPassword: TEdit
    Left = 128
    Top = 104
    Width = 310
    Height = 25
    PasswordChar = '*'
    TabOrder = 1
  end
  object btnOK: TButton
    Left = 248
    Top = 158
    Width = 90
    Height = 30
    Caption = 'OK'
    Default = True
    TabOrder = 2
    OnClick = btnOKClick
  end
  object btnCancel: TButton
    Left = 348
    Top = 158
    Width = 90
    Height = 30
    Cancel = True
    Caption = 'Annuler'
    ModalResult = 2
    TabOrder = 3
  end
end
