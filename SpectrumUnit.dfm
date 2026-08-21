object frmSpectrum: TfrmSpectrum
  Left = 0
  Top = 0
  Caption = 'ATS LAB - SPECTRUM ANALYZER'
  ClientHeight = 460
  ClientWidth = 920
  Color = 1184018
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clSilver
  Font.Height = -13
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 17
  object lblTitle: TLabel
    Left = 20
    Top = 10
    Width = 248
    Height = 25
    Caption = 'ATS LAB  -  SPECTRUM ANALYZER'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7848653
    Font.Height = -19
    Font.Name = 'Segoe UI Semibold'
    Font.Style = []
    ParentFont = False
  end
  object lblRange: TLabel
    Left = 20
    Top = 42
    Width = 245
    Height = 17
    Caption = 'En attente des donnees du scanner ATS...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object lblPeak: TLabel
    Left = 530
    Top = 42
    Width = 70
    Height = 17
    Caption = 'PIC : ---'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 8248653
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object pbSpectrum: TPaintBox
    Left = 20
    Top = 70
    Width = 880
    Height = 340
    OnPaint = pbSpectrumPaint
  end
  object btnClose: TButton
    Left = 805
    Top = 420
    Width = 95
    Height = 30
    Caption = 'FERMER'
    TabOrder = 0
    OnClick = btnCloseClick
  end
end
