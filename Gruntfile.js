'use strict';

// jshint node:true

module.exports = function(grunt) {

  var src = ['web-animations.js'];
  var tests = [];
  var support = ['Gruntfile.js'];

  var closureOptions = {
    app: {
      closureLinterPath: 'tools/closure-linter-read-only/closure_linter',
      src: [].concat(src, support, tests),
      options: {
        stdout: true,
        strict: true,
        opt: ['--nojsdoc']
      }
    }
  };

  // Project configuration.
  grunt.initConfig({
    jshint: {
      files: [].concat(src, support, tests),
      options: {
        jshintrc: '.jshintrc'
      }
    },
    closureLint: closureOptions,
    closureFixStyle: closureOptions
  });

  grunt.loadNpmTasks('grunt-contrib-jshint');
  grunt.loadNpmTasks('grunt-closure-linter');

  grunt.registerTask('lint', ['jshint', 'closureLint']);
  grunt.registerTask('fix-lint', ['closureFixStyle']);

  grunt.loadNpmTasks('grunt-contrib-yuidoc');

};
