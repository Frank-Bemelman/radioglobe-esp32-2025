<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP Web File Manager</title>
    <link rel="icon" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IB2cksfwAAAAlwSFlzAAALEwAACxMBAJqcGAAAB1xJREFUeJzlWwlsFGUUpkCFKiICmmiI1KuBKEXwRAgSjRcoilFBiQcSNQbimaixcpcgt5QCLSpHASuFSgXkCi0g5S4ttKX3fd9lu21pd9s+3zezo0vb3Zm9ZqblJV8zs52dfe/7///973/v/3v0cFCIqDdjCMOPMVwnGMYYyujrqD1KjR7FCGIkMJpIv9LCyGJsY0xi9HTV8NGMaEZrWxvRhaIG+ulEKX0alU/v7syhqeHZusC0P3Loo8g8mne0mA6kG6jJ3CYRcoUxleHlqOH9GcsYzUUGEwUeK6EHViZRjx/iugQGL75Es/7Kp4SSBmIqwEYUw1ep8XcyIlpa2ygiqYZ8lydqbpCz6L8ggZYcL6X65lb0BgxffznjBzAi0YXmcnfq/eNFzY1wB97YkUUV9WaQkMwYZo+AYDO3/PeHi8hLB4q7Ey9uzqCaRvhIOsXo05nxE+BFN8VVUa+A7tHy7fEZ+wVTq+Ag57c33gtjJLXiGt3BDkRrRT2FXjykIxJrQEA9Y4g1Aa/j09l7CzRX0tN4Yn0qWSbJn60JiDKypxy4qPu2vgT4tqNZdSCgSAiU8IdRvS2hWnPl1MIne/KkQMkPBPjiCtGd1oqphVHBKRIB70nen577LV1zxdTCrQsSJAJWgoApuHp0XYrmiqkJU4vgCsNAwDRcjQy64vBLECl++XcBJZU1UnZ1U0fUNFE5R2DVjWbK4vvQ85U0OFAfjtYohscRThPgFRBHhzMMitao1nKusJ5umR/f9QnAEtRZOZlnpJvnaUuCywRsvFDpNAGQwqvNtOFcBa04WeYWIBcwyIHh5TIBkVdqXSLAE5Jb00xPh6TduARAorPrbmwCMquabmwCMvRAwDVzK1U3mFUF4o1Cg4n2p13VjoAC9uwzInNp6LJEYWWpJvovTCAfnlpvmqMsmeN2AmobW+j+FWLm2JuVeGRtCr35eza9Ha4+ntqQSj0DVCZg9j4xmTJ8dTLFsCduaWsjrQR5TSzv+8y13RvcTgBaH90vMlkfzhGpP/RAVQjAy9DlMPYtL9aFoKCjCgExluDjmV/StbHUhmC9ogoBC2NEpmf+mde5JjJSUmeiNafLafrOHHplayZ9vCePolJqpRS2UwI/IDlljxOAwiQ+W3qi1GFFV8WW2Zy6/FmnygazUwSUGU1CaUwVAp7flCF8dtDB/ABWcfgeEivfHCgUipmI5PalXhWmUfzv2V/TqanF8Z6QWNpoV3+3EuC7XOxqYF2JoPYIQ9HyMP5ErrHDM81stJSoXX+2wmECwi/XqENAo6lVqLigfC4nyeWNQqDiY5UMmWDHcebXNgvP3sdj2armr0gWxdieAdxKQHxxg3CP1rIlUB2teBuHqyhM3Ms95vH1qUICY9beApvfQy/A1IopdlxoGu3h31QaYMGZepSAsPgq4Yd2c+CD++AztrspUmBIRSMXGHCkWKrSCsOgSsbJfc4RJspZCK+RRttxqVrWePTKIUvt72twmYDtFkWkKTBaLDd1EExHYzg2xzBBCsyZCBmbNZb+U0Z958bTPdwjDE0tdp9HZsheGOzWITDJ0tWwlaYzQXoczu6xdSkuRYltVqEtSvj2BI0hp7/bCPBblSSMbVsNezzHKDyPIMdVWRkrTpvzo0vsPrf6VLk6BMBodLXJ27JsKnO+sEFwfBO5pzgxnV8n2LaD317Ow8GevL8rVx0CKuvNsi2C8XrXksuCE7zMwYmzAoXhDDEjYCOHLcGGqNHB8mU+txBwpqBeuMba254sPlYqVJLgB2LzOgY9coLS28tbMoV3vCNTkEEJTknRxWUCDnHYG3RaHGtnC+rtKoVpCQ4MBqAnIGMzfmOaIowJSaUBi8SYHvd5HBzZk4PpBkX6u0wAanzTI8TlJoqfclLHQwGxwpiQNMEg+A6lQOD03aEiKq2TD7UlR6kKARhrd/P41pN8sFveAbqFgNh8Iz3I8T+8u14EDneEQjtcJmDtmXIhE+SMU/OUpPDs4K1WWhyLE73Jt+wnlOpvTcBbuJISEI4AS9kjmQZhH66WiCtqELbNy9UCrIGZiSUcBLyEq7GhysrK3QHevDCzSCgI8MeVvQxqdwMSLBb5GgT0Y5iwr15rxdTCa/+vXcZLW2Uvplde6zbnA+RgSd5gQdFPImAOPkEGVmvlPI2BgZekEyQHrDdLD2IYdifVCJkbrZX0JBBSk5imnNz+zEAgdk8qWUt3VYxYc4XKxdQ9TsJ5tSfAh5FYzAuOUU7EBHrH7bwAOylGrUheDrd1ZuhJRhGytf5ObJ3VK7CDBAEbiYc+Z9g7NAW8wChBUWLK9qwuf3hqZFAKnco3SsZ/xehtkwCr80NjGRkIF5HK9luVrLkhjgJFF5x+sxyXQ/N/KGt8OyJwhjCExAIN7eIZAgmQh9mR+Gi8z7czIIZBOfzVsCxae7pcmurg7Q8TToY4K/zlhxibGdcl/lC7BzN6QbuEM4Kc/YyJGNZuEX6RN2MkYyaJJ8m3MMJ0gq2MUMYXjHH/RXgK5F+OPMVpLvVbVgAAAABJRU5ErkJggg=="
    type="image/png">
    <style>
        /* Base styling */
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            margin: 0;
            padding: 0px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }

        /* Outer Card (Page as a Card) */
        .page-card {
            background-color: #fff;
            border-radius: 12px;
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2);
            width: 98vw;
            /*height: calc(100vh - 40px); /* Full height minus body padding */
            height: 92vh; /* Full height minus body padding */
            padding: 1vh;
            display: flex;
            flex-direction: column;
            overflow-y: auto; /* Enable scrolling within the card */
            position: fixed; 
            top: 1vh;
        }

        /* Header Styling */
        .header {
            background-color: #007acc;
            color: #fff;
            padding: 15px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        h1 {
            margin: 0;
            font-size: 20px;
        }

        .menu-bar button {
            padding: 10px 15px;
            background-color: #fff;
            color: #007acc;
            border: none;
            border-radius: 5px;
            font-size: 14px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        .menu-bar button:hover {
            background-color: #e0e0e0;
        }

        /* Container Styling */
        .container {
            flex-grow: 1;
            padding: .1vh;
            max-width: 98%;
            margin: 0 auto;
            overflow-y: auto; /* Enables scrolling for large content */
        }

        /* Back Button */
        #back-button {
            display: none;
            padding: 10px 15px;
            background-color: #007acc;
            color: #fff;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin-bottom: 20px;
        }

        #back-button:hover {
            background-color: #005f99;
        }

        /* File Manager Styling */
        #file-manager {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); /* Default layout */
    gap: 20px;
    margin-top: 20px;
}

@media (max-width: 768px) {
    #file-manager {
        grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); /* Adjust layout for smaller screens */
        gap: 5px;
    }
}

@media (min-width: 1200px) {
    #file-manager {
        grid-template-columns: repeat(6, 1fr); /* 6 cards for larger screens */
        /*grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));*/
        gap: 30px;
        margin-top: 20px;
    }
}

        /* Card Style for Items */
        .item {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-between;
            padding: 15px;
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }

        .item:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 12px rgba(0, 0, 0, 0.15);
        }

        .icon {
            font-size: 40px;
            margin-bottom: 10px;
            color: #007acc;
        }

        .name {
            text-align: center;
            font-size: 14px;
            margin-bottom: 10px;
            word-break: break-word;
        }

        .actions {
            display: flex;
            gap: 5%;
            width: 100%; /* Add padding inside the actions container */
            padding: 10px; /* Add padding to create space around buttons */
            justify-content: center;
        }

        .actions button {
            padding: 8px 10px;
            font-size: 12px;
            background-color: #007acc;
            color: #fff;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        .actions button:hover {
            background-color: #005f99;
        }

        /* No Files Message */
        #no-files-message {
            text-align: center;
            color: #666;
            font-size: 18px;
            padding: 20px;
            display: none;
        }

        /* Progress Modal */
        .upload-modal {
            display: none;
            position: fixed;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            background-color: #3a3a3a;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
            z-index: 1000;
        }

        .progress-container {
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .progress-circle {
            width: 100px;
            height: 100px;
            border-radius: 50%;
            background: conic-gradient(#007acc 0%, #ddd 0%);
            display: flex;
            justify-content: center;
            align-items: center;
            position: relative;
        }

        #progress-text {
            position: absolute;
            font-size: 18px;
            color: #ffffff;
        }

        /* Responsive Design */
        @media (max-width: 768px) {
            .header {
                flex-direction: column;
                text-align: center;
            }

            .menu-bar {
                margin-top: 10px;
            }

            #file-manager {
                grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            }

            .icon {
                font-size: 30px;
            }

            .name {
                font-size: 12px;
            }

            .actions button {
                font-size: 10px;
            }
        }
        /* Footer Styling (as a card) */
        .footer-card {
            margin-top: auto;
            background-color: #007acc;
            color: #fff;
            text-align: center;
            width: 98%; /* Matches the card width */
            padding: .1vh;
            border-radius: 12px 12px 12px 12px;
            box-shadow: 0 -2px 4px rgba(0, 0, 0, 0.1);
            position: fixed;         
            bottom: .5vh; /* Aligns it at the viewport bottom */
            z-index: 1000; /* Ensures it stays on top */
        }
        .footer-card a {
            color: #fff;
            text-decoration: none;
            font-weight: bold;
            transition: opacity 0.3s ease;
        }

        .footer-card a:hover {
            opacity: 0.8;
        }
        .footer-line {
            margin: 5px 0;
        }
    </style>
</head>

<body>
    <div class="page-card">
    <div class="header">
        <h1>
            <img src="data:image/png;base64,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" alt="Icon" style="width: 32px; height: 32px; vertical-align: middle; margin-right: 10px;">
            RADIOGLOBE Music Collection
        </h1>
        <div class="menu-bar">
            <button id="create-folder-button">Create Folder</button>
            <button id="upload-button">Upload Files</button>
        </div>
    </div>
    <div class="container">
        <input type="file" id="file-upload" multiple style="display: none;">
        <button id="back-button">Back</button>
        <div id="file-manager"></div>
        <div id="no-files-message">No files found.</div>
    </div>
    <div id="upload-modal" class="upload-modal">
        <div class="progress-container">
            <div class="progress-circle">
                <span id="progress-text">0%</span>
            </div>
        </div>
    </div>

    <script>
        const fileManager = document.getElementById('file-manager');
        const noFilesMessage = document.getElementById('no-files-message');
        const fileUploadInput = document.getElementById('file-upload');
        const backButton = document.getElementById('back-button');
        const uploadButton = document.getElementById('upload-button');
        const uploadModal = document.getElementById('upload-modal');
        const progressCircle = document.querySelector('.progress-circle');
        const progressText = document.getElementById('progress-text');

        let currentPath = ['/'];

        function fetchFolderContents(path) {
            fetch(`/get-folder-contents?path=${path.join('/')}`)
                .then(response => response.text())
                .then(data => {
                    const items = parseData(data);
                    displayItems(items);
                })
                .catch(error => console.error('Error fetching folder contents:', error));
        }

        function parseData(data) {
            return data.split(':').map(item => {
                const [type, name, size] = item.split(',');
                return {
                    type: type == '1' ? 'folder' : 'file',
                    name,
                    size: size === '-' ? null : parseInt(size, 10),
                };
            });
        }

        function createItemElement(item) {
    const itemElement = document.createElement('div');
    itemElement.classList.add('item');

    // Add event listener to open the folder when clicking anywhere on the card
    if (item.type === 'folder') {
        itemElement.addEventListener('click', (event) => {
            // Prevent the event if the delete button is clicked
            if (event.target.tagName === 'BUTTON') return;
            openFolder(item.name);
        });
    }

    const iconElement = document.createElement('div');
    iconElement.classList.add('icon');
    iconElement.textContent = item.type === 'folder' ? '📁' : '📄';

    const nameElement = document.createElement('div');
    nameElement.classList.add('name');
    nameElement.textContent = item.name;

    const sizeElement = document.createElement('div');
    if (item.size !== null) {
        sizeElement.textContent = `${(item.size / 1024).toFixed(2)} KB`;
        sizeElement.style.color = '#666';
    }

    itemElement.appendChild(iconElement);
    itemElement.appendChild(nameElement);
    if (item.size !== null) itemElement.appendChild(sizeElement);

    if (item.type === 'file') {
        const actionsElement = document.createElement('div');
        actionsElement.classList.add('actions');

        const downloadButton = document.createElement('button');
        downloadButton.textContent = 'Download';
        downloadButton.addEventListener('click', () => downloadFile(item.name));
        actionsElement.appendChild(downloadButton);

        const deleteButton = document.createElement('button');
        deleteButton.textContent = 'Delete';
        deleteButton.addEventListener('click', () => deleteFile(item.name));
        actionsElement.appendChild(deleteButton);

        itemElement.appendChild(actionsElement);
    } else if (item.type === 'folder') {
        const actionsElement = document.createElement('div');
        actionsElement.classList.add('actions');

        const deleteButton = document.createElement('button');
        deleteButton.textContent = 'Delete Folder';
        deleteButton.addEventListener('click', (event) => {
            event.stopPropagation(); // Prevent the folder click event from firing
            deleteFolder(item.name);
        });
        actionsElement.appendChild(deleteButton);

        itemElement.appendChild(actionsElement);
    }

    return itemElement;
}

document.addEventListener("DOMContentLoaded", function() {
        fetch('/get-fs-type')
            .then(response => response.text())
            .then(fsType => {
                if (parseInt(fsType) === 1) { // Assuming FS_SPIFFS is 1
                    document.getElementById("create-folder-button").style.display = "none";
                }
            })
            .catch(error => console.error('Error fetching filesystem type:', error));
    });

        document.getElementById('create-folder-button').addEventListener('click', () => {
        const folderName = prompt("Enter folder name:");
        if (!folderName) return;

        const folderPath = currentPath.concat(folderName).join('/');
        fetch(`/create-folder?path=${encodeURIComponent(folderPath)}`)
            .then(response => response.json())
            .then(data => {
                if (data.status === 'success') {
                    alert('Folder created successfully!');
                    fetchFolderContents(currentPath);
                } else {
                    alert(`Error creating folder: ${data.message}`);
                }
            })
            .catch(error => {
                console.error('Error creating folder:', error);
                alert('Failed to create folder.');
            });
    });

    function deleteFolder(folderName) {
        const folderPath = currentPath.concat(folderName).join('/');
        if (confirm(`Are you sure you want to delete the folder "${folderName}"?`)) {
            fetch(`/delete-folder?path=${encodeURIComponent(folderPath)}`)
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'success') {
                        alert('Folder deleted successfully!');
                        fetchFolderContents(currentPath);
                    } else {
                        alert(`Error deleting folder: ${data.message}`);
                    }
                })
                .catch(error => {
                    console.error('Error deleting folder:', error);
                    alert('Failed to delete folder.');
                });
        }
    }

        function openFolder(folderName) {
            currentPath.push(folderName);
            fetchFolderContents(currentPath);
        }

        function goBack() {
            currentPath.pop();
            fetchFolderContents(currentPath);
        }

        function displayItems(items) {
            fileManager.innerHTML = '';

            const validItems = items.filter(item => item && item.name && item.name.trim() !== '');
            if (validItems.length === 0) {
                fileManager.style.display = 'none';
                noFilesMessage.style.display = 'block';
            } else {
                fileManager.style.display = 'grid';
                noFilesMessage.style.display = 'none';

                validItems.sort((a, b) => {
                    if (a.type === b.type) {
                        return a.name.localeCompare(b.name);
                    }
                    return a.type === 'folder' ? -1 : 1;
                });

                validItems.forEach(item => {
                    const itemElement = createItemElement(item);
                    fileManager.appendChild(itemElement);
                });
            }

            backButton.style.display = currentPath.length > 1 ? 'block' : 'none';
        }

        function downloadFile(fileName) {
            const filePath = currentPath.concat(fileName).join('/');
            window.location.href = `/download?path=${filePath}`;
        }

        function deleteFile(fileName) {
            const filePath = currentPath.concat(fileName).join('/');
            fetch(`/delete?path=${filePath}`, { method: 'GET' })
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'success') {
                        alert('File deleted successfully!');
                        fetchFolderContents(currentPath);
                    } else {
                        alert(`Error deleting file: ${data.message}`);
                    }
                })
                .catch(error => {
                    console.error('Error deleting file:', error);
                    alert('Failed to delete the file.');
                });
        }

        uploadButton.addEventListener('click', () => {
            fileUploadInput.click();
        });

        fileUploadInput.addEventListener('change', () => {
            const files = fileUploadInput.files;
            if (files.length > 0) {
                uploadFiles(files);
            }
        });

        function uploadFiles(files) {
        const formData = new FormData();
        Array.from(files).forEach(file => formData.append('files[]', file));

        const currentDirectory = currentPath.join('/');
        const xhr = new XMLHttpRequest();
        xhr.open('POST', `/upload?path=${encodeURIComponent(currentDirectory)}`);
        xhr.upload.addEventListener('progress', (event) => {
            if (event.lengthComputable) {
                const percentComplete = (event.loaded / event.total) * 100;
                progressCircle.style.background = `conic-gradient(#007acc ${percentComplete}%, #ddd ${percentComplete}%)`;
                progressText.textContent = `${Math.floor(percentComplete)}%`;
                if (percentComplete === 100) {
                    setTimeout(() => {
                        uploadModal.style.display = 'none';
                        fetchFolderContents(currentPath);
                    }, 500);
                }
            }
        });

        xhr.onload = () => {
            if (xhr.status === 200) {
                alert('File uploaded successfully!');
                progressText.textContent = 'Upload complete!';
                setTimeout(() => {
                    uploadModal.style.display = 'none';
                    fetchFolderContents(currentPath);
                }, 500);
            } else {
                console.error('Upload failed:', xhr.responseText);
            }
        };

        xhr.onerror = () => {
            console.error('Upload failed.');
        };

        uploadModal.style.display = 'block';
        xhr.send(formData);
    }

        backButton.addEventListener('click', goBack);
        fetchFolderContents(currentPath);
    </script>
    </div>
<!--    
    <footer class="footer-card">
        <div class="footer-line">
            By <a href="https://github.com/jobitjoseph" target="_blank">Jobit Joseph</a>
        </div>
    </footer>
-->    
</body>

</html>