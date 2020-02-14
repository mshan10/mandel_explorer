const express = require('express')
const app = express()
const port = 3000
var cors = require('cors');
const exec = require('child_process').exec;
var bodyParser = require('body-parser')

app.use(cors({origin: 'http://localhost:8080'}));
app.use(bodyParser());

app.listen(port, () => console.log(`Example app listening on port ${port}!`))

app.post('/request',function(req,res){
    var x = req.body.x;
    var y = req.body.y;
    var s = req.body.s;
    exec(`./mandel -s ${s} -m 200 -x ${x} -y ${y} -H 512 -W 512 -n 4`, (error, stdout, stderr) => {
        if (error) {
            console.log(`error: ${error.message}`);
            return;
        }
        if (stderr) {
            console.log(`stderr: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);
        res.send("yes");
    });
});

