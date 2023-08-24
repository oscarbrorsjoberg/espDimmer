from flask import Flask, jsonify, request, render_template, url_for

app = Flask(__name__, static_url_path="/static")

intensity: int = 0
MAX_INTENSITY = 255

@app.route('/api/status', methods=['GET', 'POST'])
def get_status():
    global intensity
    if request.method == 'GET':
        response = {'message': intensity}
        return jsonify(response)
    elif request.method == 'POST':
        print(request.json)
        data = request.json
        try:
            _intens = data.get("intensity")
        except Exception as e:
            response = {"message": f"Got error {e}"}
        else:
            print(_intens)
            if(isinstance(_intens, int) or isinstance(_intens, float)):
                intensity = min(MAX_INTENSITY, _intens)
                response = {"message": f"Intensity is {intensity}"}
            else:
                response = {"message": "Intensity was not int, not set"}

        return jsonify(response)


@app.route("/")
@app.route("/home")
def home():
    return render_template('home.html', title='light intensity')

if __name__ == '__main__':
    app.run(debug=True)
