from flask import Flask, render_template, send_from_directory, request, jsonify

app = Flask(__name__, template_folder='./')

@app.route('/', methods=['GET', 'POST'])
def home():
    return render_template("color_wheel.html")

if __name__ == '__main__':
    # app.run(debug=True)
    app.run(host="0.0.0.0",port=80,debug=True)