object frmSerialMonitor: TfrmSerialMonitor
  Left = 0
  Top = 0
  Caption = 'Suivi série ATS LAB'
  ClientHeight = 320
  ClientWidth = 700
  Color = 2105376
  Font.Charset = DEFAULT_CHARSET
  Font.Color = 12632256
  Font.Height = -12
  Font.Name = 'Consolas'
  Font.Style = []
  Position = poDesigned
  OnCreate = FormCreate
  TextHeight = 14
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 700
    Height = 40
    Align = alTop
    BevelOuter = bvNone
    Color = 3158064
    ParentBackground = False
    TabOrder = 0
    object lblTitle: TLabel
      Left = 12
      Top = 11
      Width = 189
      Height = 17
      Caption = 'SUIVI DES ECHANGES SERIE'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = 8454143
      Font.Height = -13
      Font.Name = 'Consolas'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object btnClear: TButton
      Left = 602
      Top = 7
      Width = 88
      Height = 26
      Caption = 'EFFACER'
      TabOrder = 0
      OnClick = btnClearClick
    end
  end
  object memSerial: TMemo
    Left = 0
    Top = 40
    Width = 700
    Height = 280
    Align = alClient
    Color = 1052688
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 65280
    Font.Height = -12
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 1
    WordWrap = False
  end
end
