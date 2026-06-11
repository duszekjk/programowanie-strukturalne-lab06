using System.IO;
using IpcCharacterWorld;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;

public static class IpcLabSceneBuilder
{
    [MenuItem("IPC Lab/Build scene")]
    public static void BuildScene()
    {
        Directory.CreateDirectory("Assets/Scenes");
        var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);

        var floor = GameObject.CreatePrimitive(PrimitiveType.Plane);
        floor.name = "Floor";
        floor.transform.localScale = new Vector3(4, 1, 4);

        var player = GameObject.CreatePrimitive(PrimitiveType.Capsule);
        player.name = "Command Driven Character";
        player.transform.position = new Vector3(0, 1, 0);
        player.AddComponent<CharacterController>();
        var driver = player.AddComponent<CommandDrivenCharacter>();

        var receiver = new GameObject("Local Command Receiver");
        var receiverScript = receiver.AddComponent<LocalCommandReceiver>();
        receiverScript.character = driver;

        var camera = new GameObject("Main Camera");
        camera.tag = "MainCamera";
        camera.AddComponent<Camera>();
        camera.transform.position = new Vector3(0, 6, -8);
        camera.transform.rotation = Quaternion.Euler(35, 0, 0);

        var light = new GameObject("Directional Light");
        var lamp = light.AddComponent<Light>();
        lamp.type = LightType.Directional;
        light.transform.rotation = Quaternion.Euler(50, -30, 0);

        EditorSceneManager.SaveScene(scene, "Assets/Scenes/IpcLabScene.unity");
        AssetDatabase.Refresh();
    }
}
