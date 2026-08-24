object frmSpectrum: TfrmSpectrum
  Left = 0
  Top = 0
  Caption = 'ATS LAB - SPECTRUM ANALYZER'
  ClientHeight = 680
  ClientWidth = 1180
  Color = 1184018
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clSilver
  Font.Height = -13
  Font.Name = 'Georgia'
  Font.Style = []
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
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
    Top = 104
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
    Left = 540
    Top = 104
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
    Top = 132
    Width = 880
    Height = 310
    OnPaint = pbSpectrumPaint
  end
  object lblWaterfall: TLabel
    Left = 20
    Top = 450
    Width = 197
    Height = 17
    Caption = 'WATERFALL  -  SCANS RECENTS'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -13
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object pbWaterfall: TPaintBox
    Left = 20
    Top = 473
    Width = 880
    Height = 140
    OnPaint = pbWaterfallPaint
  end
  object btnClose: TButton
    Left = 1065
    Top = 632
    Width = 95
    Height = 30
    Caption = 'FERMER'
    TabOrder = 12
    OnClick = btnCloseClick
  end
  object lblStartFrequency: TLabel
    Left = 20
    Top = 51
    Width = 160
    Height = 17
    Caption = 'FREQUENCE DE DEPART'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -13
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lblStartUnit: TLabel
    Left = 300
    Top = 55
    Width = 27
    Height = 15
    Caption = 'MHz'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7848653
    Font.Height = -12
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object lblStep: TLabel
    Left = 352
    Top = 51
    Width = 28
    Height = 17
    Caption = 'PAS'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -13
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lblStepUnit: TLabel
    Left = 472
    Top = 55
    Width = 21
    Height = 15
    Caption = 'kHz'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7848653
    Font.Height = -12
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object edtStartFrequency: TEdit
    Left = 184
    Top = 47
    Width = 108
    Height = 25
    Color = 1118481
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -14
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    Text = '7.000'
  end
  object edtStep: TEdit
    Left = 390
    Top = 47
    Width = 74
    Height = 25
    Color = 1118481
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -14
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 1
    Text = '1'
  end
  object btnStart: TButton
    Left = 520
    Top = 44
    Width = 82
    Height = 32
    Caption = 'START'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 2
    OnClick = btnStartClick
  end
  object btnPause: TButton
    Left = 608
    Top = 44
    Width = 82
    Height = 32
    Caption = 'PAUSE'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 3
    OnClick = btnPauseClick
  end
  object btnResume: TButton
    Left = 696
    Top = 44
    Width = 96
    Height = 32
    Caption = 'RESTART'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 4
    OnClick = btnResumeClick
  end
  object btnStop: TButton
    Left = 798
    Top = 44
    Width = 82
    Height = 32
    Caption = 'STOP'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 5
    OnClick = btnStopClick
  end
  object btnExportCSV: TButton
    Left = 20
    Top = 632
    Width = 105
    Height = 30
    Caption = 'EXPORT CSV'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 6
    OnClick = btnExportCSVClick
  end
  object btnExportPNG: TButton
    Left = 132
    Top = 632
    Width = 105
    Height = 30
    Caption = 'EXPORT PNG'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 7
    OnClick = btnExportPNGClick
  end
  object btnFavorite: TButton
    Left = 244
    Top = 632
    Width = 112
    Height = 30
    Caption = 'FAVORIS'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 8
    OnClick = btnFavoriteClick
  end
  object lblStations: TLabel
    Left = 928
    Top = 104
    Width = 224
    Height = 17
    Alignment = taCenter
    AutoSize = False
    Caption = 'STATIONS DETECTEES'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 7848653
    Font.Height = -13
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lstStations: TListBox
    Left = 928
    Top = 132
    Width = 224
    Height = 430
    Color = 1118481
    Font.Charset = DEFAULT_CHARSET
    Font.Color = 11579568
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = []
    ItemHeight = 15
    ParentFont = False
    TabOrder = 10
    OnClick = lstStationsClick
    OnDblClick = lstStationsDblClick
  end
  object btnListen: TButton
    Left = 928
    Top = 573
    Width = 224
    Height = 40
    Caption = 'ECOUTER'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Georgia'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 11
    OnClick = btnListenClick
  end
end
