import smtplib
import datetime
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# de Def mail() functie is niet nodig, maar was handig om 'Uit te zetten' met hashtag
# Je kan input weg doen en daar je standaard Mail, WW of bericht inschrijven ipv te typen.
# Als je dit wil gebruiken, moet je bij je mail dit https://myaccount.google.com/lesssecureapps aan zetten
def mail():
    # PAS OP! Het intypen van je WW wordt niet anders weergeven, dus als iemand meekijkt kan die het zien!
    mail = 'viphuhl@gmail.com'
    ww = 'Joh@nHU1'
    msg = MIMEMultipart()

    msg['From'] = mail
    msg['To'] = mail
    msg['Subject'] = input("Mail onderwerp: ")

    body = 'Je alarm is afgegaan!' + "\n\nVerzonden op {}".format(
        datetime.datetime.now().strftime("%d %B %Y %H:%M."))# Tijd wanneer verzonden

    msg.attach(MIMEText(body, 'plain'))

    server_ssl = smtplib.SMTP_SSL('smtp.gmail.com', 465)
    server_ssl.ehlo()
    server_ssl.login(mail, ww)
    text = msg.as_string()
    server_ssl.sendmail(mail, mail, text)  # Stuurt naar zichzelf
    server_ssl.quit()

mail()
