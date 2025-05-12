import pandas as pd
import numpy as np

import os

def drop_columns_with_zeroes(df: pd.DataFrame) -> pd.DataFrame:
    df = df[~(df.iloc[:, 2:] == 0).all(axis=1)].reset_index(drop=True)
    return df

def read_instance(file_path: str) -> pd.DataFrame:
    
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"The file {file_path} does not exist.")
    try:
        df = pd.read_excel(file_path, header=None,sheet_name=None, converters={'A': str})
    except Exception as e:
        raise ValueError(f"Error reading the file {file_path}: {e}")

    df['PASSAGEM'] = df['PASSAGEM'].append(df['PASSAGEM2'].iloc[1:], ignore_index=True)
    df.pop('PASSAGEM2')
    
    df['ROTAS2'] = df['ROTAS2'].append(df['ROTAS3'].iloc[1:], ignore_index=True)
    df.pop('ROTAS3')

    df['CASK'] = df['CASK'].append(df['CASK2'].iloc[1:], ignore_index=True)
    df.pop('CASK2')
    
    df['CASK'] = drop_columns_with_zeroes(df['CASK'])
    df['PASSAGEM'] = drop_columns_with_zeroes(df['PASSAGEM'])
    df['ROTAS'] = df['ROTAS'][~(df['ROTAS'].iloc[:, 3:] == 0).all(axis=1)].reset_index(drop=True)
    
    all_cols = df['ROTAS2'].columns.tolist()

    cols_to_check = [i for i in all_cols if i not in [0,1,2,4]]
    
    df['ROTAS2'] = df['ROTAS2'][~(df['ROTAS2'].iloc[:, cols_to_check] == 0).all(axis=1)].reset_index(drop=True)
    
    df = {k: v for k, v in df.items() if k in ['ROTAS', 'ROTAS2', 'PASSAGEM', 'CASK', 'AERONAVE']}
    return df 

def create_csv_for_each_sheet(file_path: str) -> None:
    df = read_instance(file_path)
    
    for sheet_name, data in df.items():
        csv_file_path = f"../../data/processed/{sheet_name}.csv"

        with open(csv_file_path, 'w') as f:
            data.to_csv(f, index=None, header = None, sep=';', line_terminator='\n')
            print(f"Created {csv_file_path} from {sheet_name} sheet.")
        

create_csv_for_each_sheet("../../data/DadosTCCv2.xlsx")
