## Roadmap

### Блок 1 — Рендер Core
```
[✓] 01. Win32 окно
[✓] 02. DirectX 11 Device + SwapChain
[✓] 03. Логгер
[✓] 04. Vertex / Index Buffer
[✓] 05. HLSL шейдеры (VS + PS)
[✓] 06. Камера + матрицы MVP
[✓] 07. Depth Buffer + Rasterizer State
[✓] 08. Cube Mesh (фабричный метод)
[✓] 09. Input система (клавиатура + мышь — RawInput)
[✓] 10. Wireframe режим по кнопке (горячая клавиша F1)
```

### Блок 2 — Материалы и текстуры
```
[✓] 11. UV координаты в вершинах
[✓] 12. Texture2D загрузка (stb_image, PNG/JPG/BMP)
[✓] 13. Sampler State (point / bilinear / trilinear / anisotropic)
[✓] 14. Базовый текстурный шейдер
[✓] 15. Multi-texture (diffuse + specular map в одном draw)
[✓] 16. Transparency (alpha blending, blend state)
[✓] 17. Cubemap (skybox текстура)
[✓] 18. Skybox рендеринг (отдельный шейдер, depth = 1.0)
```

### Блок 3 — Освещение
```
[✓] 19. Нормали в вершинах + пересчёт в шейдере
[✓] 20. Ambient освещение (глобальная константа)
[✓] 21. Directional Light (бесконечно далёкий, направление)
[✓] 22. Diffuse Lambert (dot(N, L))
[✓] 23. Specular Phong (reflect + dot)
[✓] 24. Specular Blinn-Phong (half-vector, быстрее)
[✓] 25. Point Light (затухание 1/d²)
[✓] 26. Spot Light (конус, inner/outer angle)
[✓] 27. Light Buffer (массив до 16 источников в один CB)
[✓] 28. Normal Map (TBN матрица, tangent space)
```

### Блок 4 — Архитектура сцены
```
[✓] 29. Transform компонент (pos + rot + scale → Matrix)
[✓] 30. Entity (уникальный ID + список компонентов)
[✓] 31. Component базовый класс (onUpdate, onRender)
[✓] 32. Scene (реестр Entity, update + render loop)
[✓] 33. SceneHierarchy (parent-child трансформации)
[✓] 34. MeshRenderer компонент
[✓] 35. Material система (шейдер + параметры + текстуры)
[✓] 36. Загрузчик OBJ мешей (tinyobjloader)
[x] 37. Загрузчик GLTF мешей (tinygltf)
[✓] 38. Bounding Box (AABB для каждого меша)
```

### Блок 5 — GPU оптимизации
```
[✓] 39. Per-object Constant Buffer (каждый Entity свои матрицы)
[✓] 40. Frustum Culling (AABB vs 6 плоскостей камеры)
[✓] 41. Instancing (DrawIndexedInstanced, трава/деревья)
[✓] 42. Depth Pre-pass (Z-only pass перед основным)
[✓] 43. Occlusion Culling (GPU query)
[✓] 44. LOD система (Level of Detail по дистанции)
```

### Блок 6 — Shadow
```
[✓] 45. Shadow Map (рендер с позиции света в текстуру)
[✓] 46. PCF фильтрация теней (мягкие края)
[✓] 47. Cascade Shadow Maps (CSM для больших сцен)
[✓] 48. Shadow Bias (убрать shadow acne)
```

### Блок 7 — Постпроцессинг
```
[✓] 49. Render To Texture (рендерить сцену в текстуру)
[✓] 50. Fullscreen Quad (квад для постэффектов)
[✓] 51. Gamma Correction (линейное → sRGB)
[✓] 52. Tone Mapping (HDR → LDR, Reinhard / ACES)
[ ] 53. FXAA (быстрое сглаживание пост-процессом)
[ ] 54. MSAA (мультисэмплинг на уровне SwapChain)
[ ] 55. Bloom (bright pass + gaussian blur + composite)
[ ] 56. SSAO (Screen Space Ambient Occlusion)
[ ] 57. Motion Blur (velocity buffer)
[ ] 58. Depth of Field (размытие по глубине)
[ ] 59. Color Grading (LUT текстура 3D)
[ ] 60. Vignette + Chromatic Aberration
```

### Блок 8 — Asset система
```
[ ] 61. Asset Handle (UUID + слабая ссылка)
[ ] 62. Asset Manager (загрузка, кэш, ref counting)
[ ] 63. Бинарный формат мешей (.smesh, быстрее OBJ)
[ ] 64. Asset Import Pipeline (OBJ/GLTF → .smesh)
[ ] 65. Асинхронная загрузка (std::future + thread pool)
[ ] 66. Hot Reload шейдеров (FileWatcher + перекомпиляция)
[ ] 67. Hot Reload текстур
[ ] 68. Virtual File System (пути через алиасы assets://)
```

### Блок 9 — Физика
```
[ ] 69. Коллайдеры (Box, Sphere, Capsule — структуры данных)
[ ] 70. Интеграция PhysX или Jolt Physics
[ ] 71. RigidBody компонент
[ ] 72. Collision Callbacks (onCollisionEnter/Stay/Exit)
[ ] 73. Raycasting (мышь → мир, picking объектов)
[ ] 74. CharacterController (капсула + движение без физики)
```

### Блок 10 — Audio
```
[ ] 75. Интеграция XAudio2 (Windows native)
[ ] 76. AudioClip (загрузка WAV/OGG)
[ ] 77. AudioSource компонент (play/pause/stop/loop)
[ ] 78. 3D Audio (позиционный звук, затухание)
[ ] 79. AudioMixer (каналы, volume, pitch)
```

### Блок 11 — Qt Editor
```
[ ] 80. Qt интеграция — DX11 viewport в QWindow::winId()
[ ] 81. Rider / CLion plugin манифест (.idea структура)
[ ] 82. MainWindow (меню, toolbar, dockable панели)
[ ] 83. Viewport панель (рендер движка внутри Qt)
[ ] 84. Scene Hierarchy (QTreeView → список Entity)
[ ] 85. Inspector панель (компоненты выбранного Entity)
[ ] 86. Asset Browser (файловая система проекта)
[ ] 87. Drag & drop ассетов из браузера в сцену
[ ] 88. Gizmos — Translation (стрелки перемещения)
[ ] 89. Gizmos — Rotation (дуги вращения)
[ ] 90. Gizmos — Scale (кубики масштаба)
[ ] 91. Play / Pause / Stop режим в редакторе
[ ] 92. Сохранение/загрузка сцены (JSON формат)
[ ] 93. Undo / Redo система (Command паттерн)
```

### Блок 12 — Скриптинг C#
```
[ ] 94. Встраивание .NET CoreCLR (hostfxr)
[ ] 95. C API — экспорт функций движка в C#
[ ] 96. ScriptComponent — C# класс на Entity
[ ] 97. Bindings — Transform, Input, Physics из C#
[ ] 98. Hot Reload скриптов без перезапуска
[ ] 99. Debugger поддержка (attach Rider к C# скриптам)
[100. ] Publish Build (strip editor, упаковка ассетов, exe)
```