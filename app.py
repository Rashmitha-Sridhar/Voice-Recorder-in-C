from flask import Flask, render_template, request, redirect, url_for, send_from_directory
import subprocess
import os
import glob

app = Flask(__name__)

# Configuration
RECORDINGS_FOLDER = "Recordings"
VOICE_RECORDER_EXE = "voice_recorder.exe"  # Assuming you compile the C program to this executable

# Create recordings folder if it doesn't exist
if not os.path.exists(RECORDINGS_FOLDER):
    os.makedirs(RECORDINGS_FOLDER)

@app.route('/')
def index():
    # Get list of recordings (sorted by newest first)
    recordings = glob.glob(os.path.join(RECORDINGS_FOLDER, "*.wav"))
    recordings.sort(key=os.path.getmtime, reverse=True)
    
    # Extract just the filenames for display
    recording_files = [os.path.basename(path) for path in recordings]
    
    return render_template('index.html', recordings=recording_files)

@app.route('/record', methods=['POST'])
def record():
    duration = request.form.get('duration', type=int)
    
    if duration and duration > 0:
        try:
            # Create a process pipe to send input to the C program
            process = subprocess.Popen([VOICE_RECORDER_EXE], 
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      text=True)
            
            # Send the duration to the program
            stdout, stderr = process.communicate(input=f"{duration}\n")
            
            # For debugging
            print(f"Output: {stdout}")
            if stderr:
                print(f"Error: {stderr}")
                
            return redirect(url_for('index'))
        except Exception as e:
            return render_template('error.html', error=str(e))
    else:
        return render_template('error.html', error="Invalid duration")

@app.route('/play/<filename>')
def play_recording(filename):
    return render_template('player.html', filename=filename)

@app.route('/download/<filename>')
def download_recording(filename):
    return send_from_directory(RECORDINGS_FOLDER, filename, as_attachment=True)

@app.route('/recordings/<filename>')
def serve_recording(filename):
    return send_from_directory(RECORDINGS_FOLDER, filename)

if __name__ == '__main__':
    app.run(debug=True)