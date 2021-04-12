

import gspread
 
from oauth2client.service_account import ServiceAccountCredentials
 
from datetime import datetime
 
import time
 

 
#carrega as credenciais para uso das APIs do Google
 
scope = ["https://spreadsheets.google.com/feeds","https://www.googleapis.com/auth/spreadsheets","https://www.googleapis.com/auth/drive.file","https://www.googleapis.com/auth/drive"]
 
creds = ServiceAccountCredentials.from_json_keyfile_name('TestSheets-a46ab399d19e.json', scope)
 
client = gspread.authorize(creds)
 
 
 
#Informa a planilha do Google Sheets a ser acessada
 
#Sera considerado que todas as informacoes serao escritas somente na primeira aba da planilha
 
sheet = client.open('teste').sheet1
 
 
 
#Por tempo indeterminado,faz:
 
#- leitura do sensor
 
#- envio das informacoes do sensor (juntamente com a data e hora da medicao) para a planilha do Google Sheets
 
#- aguarda um minuto para a proxima medicao
umid = 0
temp = 0
while True:
 
#obtem a data e hora atuais
 
	now = datetime.now()
 
	datahora = now.strftime("%d/%m/%Y %H:%M:%S")
 
 
 
#faz leitura do sensor
 
	umid= umid+1
	temp= temp+1
 
	if umid is not None and temp is not None:
 
#envia data, hora e informacoes do sensor para Google Sheets
 
		linha_a_ser_adicionada = [datahora, str(temp), str(umid)]
 
		sheet.append_row(linha_a_ser_adicionada)
 
 
 
#aguarda um minuto
 
	time.sleep(60)