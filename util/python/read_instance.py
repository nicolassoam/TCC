import pandas as pd
import numpy as np

import os

def read_instance(file_path: str) -> pd.DataFrame:
    
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"The file {file_path} does not exist.")
    try:
        df = pd.read_excel(file_path, header=None,sheet_name=None, converters={'A': str})
    except Exception as e:
        raise ValueError(f"Error reading the file {file_path}: {e}")
    # filter to keep only first 6 sheets
   
    df = {k: v for k, v in df.items() if k in ['ROTAS', 'ROTAS2', 'ROTAS3', 'PASSAGEM', 'PASSAGEM2', 'CASK']}
    
    return df 

print(read_instance("../../data/DadosTCCv2.xlsx")['ROTAS'])
