import { Template } from 'meteor/templating';
import { ReactiveVar } from 'meteor/reactive-var';

import './main.html';
//import './api.html';

Router.route('/', {
    name: 'Home',
    template: 'Home'
});

Router.route('/api', {
    name: 'api',
    template: 'api'
});



