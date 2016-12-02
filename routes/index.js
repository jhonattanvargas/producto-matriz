'use strict'

const express = require('express')
const api = express.Router()
//add require controller
const calculatorCtrl = require('../controllers/calculator')

//add routes from api
api.get('/serial/:n',calculatorCtrl.serial)
api.get('/parallel/:n',calculatorCtrl.parallel)
api.post('/test',calculatorCtrl.test)

module.exports = api
