'use strict'

const express = require('express')
const bodyParser = require('body-parser')
const app = express()
const api = require('./routes')

app.use(bodyParser.urlencoded({extended:false,limit: '50mb'}))
app.use(bodyParser.json({limit: '50mb'}))
app.use('/api', api)

module.exports = app
