#%%
import pandas as pd
import numpy as np

# %%
df = pd.read_csv('base/INMET_SE_SP_A701_SAO PAULO - MIRANTE_01-01-2024_A_31-12-2024.CSV', sep=';', encoding='latin-1')

#%%
colunas = [
       'Data', 
       'Hora UTC', 
       'TEMPERATURA DO AR - BULBO SECO, HORARIA (°C)',
       'UMIDADE RELATIVA DO AR, HORARIA (%)',
       'TEMPERATURA MÍNIMA NA HORA ANT. (AUT) (°C)'
]
df = df[colunas]

df['DIA'] = df['Data'].str[8:]
df['MES'] = df['Data'].str[5:-3]
df['HORA'] = df['Hora UTC'].str[:-6]
df = df.drop(['Data','Hora UTC'], axis=1)

#%%
df['TEMPERATURA'] = df['TEMPERATURA DO AR - BULBO SECO, HORARIA (°C)'].str.replace(',', '.').astype(float)
df['TEMPERATURA_MINIMA_HORA_ANTERIOR'] = df['TEMPERATURA MÍNIMA NA HORA ANT. (AUT) (°C)'].str.replace(',', '.').astype(float)
df['UMIDADE'] = df['UMIDADE RELATIVA DO AR, HORARIA (%)']

#%%
df['DIA'] = df['DIA'].str.replace(',', '.').astype(float).astype(int)
df['MES'] = df['MES'].str.replace(',', '.').astype(float).astype(int)
df['HORA'] = df['HORA'].str.replace(',', '.').astype(float).astype(int)

#%%
df = df.drop(columns=[
    'TEMPERATURA DO AR - BULBO SECO, HORARIA (°C)',
    'TEMPERATURA MÍNIMA NA HORA ANT. (AUT) (°C)',
    'UMIDADE RELATIVA DO AR, HORARIA (%)'], axis=1)
df = df.dropna()

#%%
df['PRIMAVERA'] = (df['MES'].isin([9, 10, 11])).astype(int)
df['VERAO'] = (df['MES'].isin([12, 1, 2])).astype(int)
df['OUTONO'] = (df['MES'].isin([3, 4, 5])).astype(int)
df['INVERNO'] = (df['MES'].isin([6, 7, 8])).astype(int)

#%%
df['DIURNO'] = np.where(df['HORA'].between(6, 18), 1, 0)
df['NOTURNO'] = np.where((df['HORA'] >= 19) | (df['HORA'] <= 5), 1, 0)

#%%
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, r2_score, mean_absolute_error

y = df['TEMPERATURA']
X = df[['UMIDADE','HORA','PRIMAVERA','VERAO','OUTONO','INVERNO','DIURNO']]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, shuffle=False)

model = LinearRegression()
model.fit(X_train, y_train)
y_pred = model.predict(X)

mse = mean_squared_error(y, y_pred)
rmse = np.sqrt(mse)
mae = mean_absolute_error(y, y_pred)
r2 = r2_score(y, y_pred)

print(f"\n=== RESULTADOS DO MODELO ===")
print(f"Coeficientes: {model.coef_}")
print(f"Intercepto: {model.intercept_:.6f}")
print(f"R²: {r2:.2f}")
print(f"MSE: {mse:.2f}")
print(f"RMSE: {rmse:.2f}")
print(f"MAE: {mae:.2f}")