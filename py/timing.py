# seconds per 1e3 iterations
# dict[type][dimension][N]

time = {1: { 2:
             {   32:   0.37,
                 64:   1.35,
                128:   5.28,
                256:  21.12,
                362:  23.44,
                512:  56.29,
                724: 103,
               1024: 210,
               1448: 470,
               2048: 820,
             }
           },
        2: { 2:
             {   32:    10,
                 64:    40,
                128:   210,
                256:   850,
                512:  3920,
               1024: 15000,
               2048: 45000,
             },
             3:
             {   32:    10,
                 64:    40,
                128:   210,
                256:   850,
                512:  3920,
               1024: 15000,
               2048: 45000,
             }
           },
        3: { 2:
             {   32:    0.18,
                 64:    0.66,
                128:    3.20,
                256:   13.16,
                512:   51.79,
               1024:  400.0,
               1448:  780.0,
               2048: 2660.0,

             },
             3:
             {
                 64:   2.3,
                128:  16.7,
                256:  14.13,
                512:  100
             }
           },
        5: { 2:
             {   32:  0.16,
                 64:  3.85,
                128: 17.25,
                256:  5.27,
                512: 16.07
             },
             3:
             {
                 64: 11.17,
                128: 16.32
             }
           }
       }
