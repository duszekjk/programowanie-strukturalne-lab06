using System;
using System.Collections.Concurrent;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

namespace IpcCharacterWorld
{
    public sealed class LocalCommandReceiver : MonoBehaviour
    {
        public CommandDrivenCharacter character;
        public int udpPort = 7777;
        public string fifoPath = "/tmp/unity_ipc_character_commands";

        private readonly ConcurrentQueue<CharacterCommand> commands = new ConcurrentQueue<CharacterCommand>();
        private Thread udpThread;
        private Thread fifoThread;
        private volatile bool running;
        private UdpClient udpClient;

        private void Start()
        {
            if (character == null)
            {
                character = FindObjectOfType<CommandDrivenCharacter>();
            }

            running = true;
            udpThread = new Thread(UdpLoop) { IsBackground = true };
            fifoThread = new Thread(FifoLoop) { IsBackground = true };
            udpThread.Start();
            fifoThread.Start();
        }

        private void Update()
        {
            while (commands.TryDequeue(out CharacterCommand command))
            {
                if (character != null)
                {
                    character.Execute(command);
                }
            }
        }

        private void OnDestroy()
        {
            running = false;
            try { udpClient?.Close(); } catch { }
        }

        private void EnqueueLine(string line)
        {
            if (CommandParser.TryParse(line, out CharacterCommand command))
            {
                commands.Enqueue(command);
            }
            else if (!string.IsNullOrWhiteSpace(line))
            {
                Debug.LogWarning("Unknown command: " + line);
            }
        }

        private void UdpLoop()
        {
            try
            {
                udpClient = new UdpClient(udpPort);
                IPEndPoint remote = new IPEndPoint(IPAddress.Any, 0);

                while (running)
                {
                    byte[] data = udpClient.Receive(ref remote);
                    string text = System.Text.Encoding.UTF8.GetString(data);
                    foreach (string line in text.Split('\n'))
                    {
                        EnqueueLine(line);
                    }
                }
            }
            catch (ObjectDisposedException) { }
            catch (SocketException ex)
            {
                Debug.LogWarning("UDP receiver stopped: " + ex.Message);
            }
            catch (Exception ex)
            {
                Debug.LogWarning("UDP receiver error: " + ex.Message);
            }
        }

        private void FifoLoop()
        {
            while (running)
            {
                try
                {
                    if (!File.Exists(fifoPath))
                    {
                        Thread.Sleep(500);
                        continue;
                    }

                    using (FileStream stream = new FileStream(fifoPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                    using (StreamReader reader = new StreamReader(stream))
                    {
                        while (running)
                        {
                            string line = reader.ReadLine();
                            if (line == null)
                            {
                                break;
                            }
                            EnqueueLine(line);
                        }
                    }
                }
                catch (Exception ex)
                {
                    Debug.LogWarning("FIFO receiver waiting: " + ex.Message);
                    Thread.Sleep(500);
                }
            }
        }
    }
}
