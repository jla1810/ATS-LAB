object frmFrequencyInput: TfrmFrequencyInput
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Entrer la frequence'
  ClientHeight = 170
  ClientWidth = 420
  Position = poScreenCenter
  object lblTitle: TLabel
    Left = 24
    Top = 20
    Width = 130
    Height = 17
    Caption = 'Frequence'
  end
  object lblHint: TLabel
    Left = 24
    Top = 48
    Width = 340
    Height = 17
    Caption = 'Ex: 7.100 / 101.700 / 7100'
  end
  object edtFrequency: TEdit
    Left = 24
    Top = 76
    Width = 372
    Height = 25
    TabOrder = 0
  end
  object btnOK: TButton
    Left = 216
    Top = 120
    Width = 84
    Height = 30
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = btnOKClick
  end
  object btnCancel: TButton
    Left = 312
    Top = 120
    Width = 84
    Height = 30
    Cancel = True
    Caption = 'Annuler'
    ModalResult = 2
    TabOrder = 2
  end
end
