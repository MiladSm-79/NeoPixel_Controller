import 'package:flutter/material.dart';
import 'package:neo_pixel/AppSettings.dart';
import 'package:neo_pixel/theme/Colors.dart';
import 'package:neo_pixel/theme/fonts.dart';

class SettingsPage extends StatefulWidget {
  const SettingsPage({super.key});

  @override
  State<SettingsPage> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  final _formKey = GlobalKey<FormState>();
  final s = AppStorage.instance;

  late final TextEditingController _ipCtrl;
  late final TextEditingController _portCtrl;
  late final TextEditingController _pixelCtrl;

  double _offset = 0;

  @override
  void initState() {
    super.initState();
    _ipCtrl = TextEditingController(text: s.ip);
    _portCtrl = TextEditingController(text: s.port);
    _pixelCtrl = TextEditingController(text: s.pixelCount.toString());
    _offset = s.ntpOffsetHours;
  }

  String? _validateInt(String? v, {required int min, required int max, required String name}) {
    if (v == null || v.isEmpty) return "Enter $name";
    final n = int.tryParse(v);
    if (n == null || n < min || n > max) return "$name must be $min–$max";
    return null;
  }

  Future<void> _save() async {
    if (!_formKey.currentState!.validate()) return;

    s
      ..ip = _ipCtrl.text.trim()
      ..port = _portCtrl.text.trim()
      ..ntpOffsetHours = _offset
      ..pixelCount = int.parse(_pixelCtrl.text);

    await s.save();
    if (mounted) Navigator.pop(context);
  }

  @override
  Widget build(BuildContext context) {
    final offsets = List.generate(53, (i) => -12 + i * 0.5);

    return Scaffold(
      appBar: AppBar(title: Text("Settings", style: Fonts.smallTextStyle)),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: ListView(
            children: [
              _buildField(_ipCtrl, "Device IP"),

              const SizedBox(height: 12),

              TextFormField(
                controller: _portCtrl,
                keyboardType: TextInputType.number,
                decoration: _decor("Port"),
                validator: (v) => _validateInt(v, min: 1, max: 65535, name: "Port"),
              ),

              const SizedBox(height: 12),

              TextFormField(
                controller: _pixelCtrl,
                keyboardType: TextInputType.number,
                decoration: _decor("LED Pixel Count"),
                validator: (v) => _validateInt(v, min: 1, max: 512, name: "Pixel Count"),
              ),

              const SizedBox(height: 12),

              Row(
                children: [
                  Text("Timezone Offset: ", style: Fonts.smallTextStyle),
                  DropdownButton<double>(
                    value: _offset,
                    items: offsets
                        .map((e) => DropdownMenuItem(
                              value: e,
                              child: Text(e > 0 ? "+$e" : "$e", style: Fonts.smallTextStyle),
                            ))
                        .toList(),
                    onChanged: (v) => setState(() => _offset = v!),
                  ),
                ],
              ),

              const SizedBox(height: 30),

              ElevatedButton(
                onPressed: _save,
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.symmetric(vertical: 14),
                  backgroundColor: ProjectColors.buttonColor,
                  foregroundColor: ProjectColors.defaultWhiteColor,
                ),
                child: Text("Save", style: Fonts.smallTextStyle),
              ),
            ],
          ),
        ),
      ),
    );
  }

  TextFormField _buildField(TextEditingController c, String label) {
    return TextFormField(
      controller: c,
      decoration: _decor(label),
      validator: (v) => (v == null || v.isEmpty) ? "Enter $label" : null,
    );
  }

  InputDecoration _decor(String label) => InputDecoration(
        labelText: label,
        labelStyle: Fonts.smallTextStyle,
        border: const OutlineInputBorder(),
      );
}
