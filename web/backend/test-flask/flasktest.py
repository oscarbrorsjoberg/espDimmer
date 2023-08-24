from flask import Flask, render_template
app = Flask(__name__)

posts = [
    {
        'author': 'blah blah',
        'title': 'Blog post 1',
        'content': 'First post blah',
        'date_posted': 'April 20, 2018',
    },
    {
        'author': 'blah da',
        'title': 'Blog post 2',
        'content': 'second post blah',
        'date_posted': 'April 21, 2018',
    },
]


@app.route("/")
@app.route("/home")
def home():
    return render_template('home.html', posts=posts)

@app.route("/about")
def about():
    return render_template('about.html', title='Neuman')

if __name__ == '__main__':
    app.run(debug=True)

