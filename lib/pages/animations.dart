import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:neo_pixel/AppSettings.dart';

class Animations extends StatefulWidget {
  const Animations({super.key});

  @override
  State<Animations> createState() => _AnimationsState();
}

class _AnimationsState extends State<Animations> {
  final List<String> animations = [
    "None",
    "Rainbow",
    "NoisyFlow",
    "Pulse",
    "RainDrop",
    "MirrorFill",
    "Comet",
    "BonFire",
    "Aurora",
    "Firefly",
    "OceanWave",
    "PlasmaWarp",
    "Gradient",
    "CandleFlicker",
    "Cloud",
    "LavaLamp",
    "Sunrise",
    "MatrixRain",
    "ColorCycle"
  ];

  int? selectedIndex;
  
  final s = AppStorage.instance;
  
  Future<void> _onClick(int index) async {
    try {
      final url = Uri.parse("http://${s.ip}:${s.port}/?mode=$index");
      await http.get(url);
    } catch (e) {
      print(e);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Column(
          children: [
            Padding(
              padding: const EdgeInsets.all(24.0),
              child: Align(
                alignment: Alignment.center,
                child: Text(
                  "Animations",
                  style: TextStyle(
                    fontFamily: "Poppins",
                    fontSize: 32,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ),
            Expanded(
              child: ListView.builder(
                padding: const EdgeInsets.all(8),
                itemCount: animations.length,
                itemBuilder: (context, index) {
                  return Padding(
                    padding: const EdgeInsets.all(12.0),
                    child: InkWell(
                      onTap: () {
                        setState(() {
                          selectedIndex = index;
                        });
                        _onClick(index); 
                      },
                      child: Container(
                        height: 60,
                        decoration: BoxDecoration(
                          color: Colors.grey.shade300,
                          borderRadius: BorderRadius.circular(12),
                        ),
                        child: Padding(
                          padding: const EdgeInsets.symmetric(horizontal: 24),
                          child: Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              Text(
                                animations[index],
                                style: TextStyle(
                                  fontFamily: "Poppins",
                                  fontSize: 18,
                                  fontWeight: FontWeight.w500,
                                ),
                              ),
                              Checkbox(
                                value: selectedIndex == index,
                                onChanged: (value) {
                                  setState(() {
                                    selectedIndex = value == true ? index : null;
                                  });
                                  _onClick(index);
                                },
                              ),
                            ],
                          ),
                        ),
                      ),
                    ),
                  );
                },
              ),
            ),
            SizedBox(height: 24,)

          ],
        ),
      ),
    );
  }
}
