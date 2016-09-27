object FPSPatcher: TFPSPatcher
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'FPS AutoPatcher v2.0'
  ClientHeight = 254
  ClientWidth = 409
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object l_status: TLabel
    Left = 311
    Top = 49
    Width = 44
    Height = 13
    Caption = 'Unknown'
  end
  object Label1: TLabel
    Left = 128
    Top = 8
    Width = 43
    Height = 13
    Caption = 'Client list'
  end
  object Label2: TLabel
    Left = 311
    Top = 30
    Width = 35
    Height = 13
    Caption = 'Status:'
  end
  object bt_patch: TButton
    Left = 311
    Top = 84
    Width = 89
    Height = 25
    Caption = 'Set patch'
    TabOrder = 0
    OnClick = bt_patchClick
  end
  object bt_refresh: TButton
    Left = 311
    Top = 115
    Width = 89
    Height = 25
    Caption = 'Refresh'
    TabOrder = 1
    OnClick = bt_refreshClick
  end
  object lb_client_list: TListBox
    Left = 8
    Top = 27
    Width = 297
    Height = 219
    ItemHeight = 13
    TabOrder = 2
    OnClick = lb_client_listClick
  end
end
