import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';

class ColorPickerSection extends StatelessWidget {
  final Color pickerColor;
  final ValueChanged<Color> onChanged;
  final VoidCallback? onChangeEnd; // change to callback with no color argument

  const ColorPickerSection({
    super.key,
    required this.pickerColor,
    required this.onChanged,
    this.onChangeEnd,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onPanEnd: (_) => onChangeEnd?.call(), // detect release of drag
      child: ColorPicker(
        pickerColor: pickerColor,
        onColorChanged: onChanged,
        enableAlpha: false,
        paletteType: PaletteType.hueWheel,
      ),
    );
  }
}
