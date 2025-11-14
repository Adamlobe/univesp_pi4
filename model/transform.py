#%%
import pickle
import json
import pandas as pd

#%%
def read_concatenated_json(file_path):

    objects = []
    
    with open(file_path, 'r') as file:
        content = file.read().strip()
        
        # Encontra todos os objetos JSON completos
        start = 0
        depth = 0
        in_string = False
        escape_next = False
        
        for i, char in enumerate(content):
            # Controle de strings
            if not escape_next:
                if char == '"' and not in_string:
                    in_string = True
                elif char == '"' and in_string:
                    in_string = False
                elif char == '\\' and in_string:
                    escape_next = True
            else:
                escape_next = False
            
            # Controle de profundidade de objetos
            if not in_string:
                if char == '{':
                    if depth == 0:
                        start = i  # Marca início do objeto
                    depth += 1
                elif char == '}':
                    depth -= 1
                    if depth == 0:
                        # Encontrou um objeto completo
                        json_str = content[start:i+1]
                        try:
                            obj = json.loads(json_str)
                            objects.append(obj)
                        except json.JSONDecodeError as e:
                            print(f"Erro ao decodificar: {e}")
    
    return objects
#%%
arquivo = read_concatenated_json('emulation.json')

df = pd.DataFrame(arquivo)

#%%
df.to_parquet("leitura.parquet", index=False)

#%%
with open('modelo_LinearRegression.pkl', 'wb') as f:
    pickle.dump(model, f)

with open('modelo.pkl', 'rb') as f:
    model_carregado = pickle.load(f)



