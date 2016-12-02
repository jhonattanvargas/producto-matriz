{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "addon.cc" ],
      "conditions":[ ['OS=="linux"', {
            'cflags': [
              '-w'
              ]},
             {# OS != "linux"
               'cflags': [
                 '-w'
                 ]},
          ],
         ]
    }
  ],
  "link_settings":
    {
      "libraries": [
        "-pthread"
      ]
    }
}
