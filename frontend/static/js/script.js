document.addEventListener('DOMContentLoaded', () => {
    const generateBtn = document.getElementById('generate-btn');
    const statusMsg = document.getElementById('status-message');
    const resultsSection = document.getElementById('results-section');
    const timetableDisplay = document.getElementById('timetable-display');
    const periodSelect = document.getElementById('period-select');
    const periodsData = JSON.parse(document.getElementById('periods-data').textContent);
    const timeLimitSelect = document.querySelector('select:nth-of-type(1)'); // Assuming first select in Generate section

    // Update Info Panel on Period Change
    function updatePeriodInfo() {
        const selectedPeriod = periodSelect.value;
        const info = periodsData[selectedPeriod];

        // Update Period Name (mapped manually or from data if available)
        const periodNames = {
            "may-aug": "Mayo - Agosto",
            "sept-dec": "Septiembre - Diciembre",
            "jan-apr": "Enero - Abril"
        };

        document.querySelector('.info-row:nth-child(1) span:nth-child(2)').textContent = periodNames[selectedPeriod] || selectedPeriod;

        // Update Semesters
        const badgesContainer = document.querySelector('.info-row:nth-child(2) div');
        badgesContainer.innerHTML = '';
        const semesters = Object.keys(info.cuatrimestres).sort((a, b) => a - b);
        semesters.forEach(sem => {
            const badge = document.createElement('span');
            badge.className = 'badge';
            badge.textContent = `${sem}°`;
            badgesContainer.appendChild(badge);
        });
    }

    periodSelect.addEventListener('change', updatePeriodInfo);
    // Initialize
    updatePeriodInfo();

    generateBtn.addEventListener('click', async () => {
        statusMsg.textContent = "Generando horario... Esto puede tardar unos segundos.";
        statusMsg.style.color = "#fbbf24"; // Yellow

        const period = periodSelect.value;

        // Parse timeout
        let timeout = 60;
        const timeVal = timeLimitSelect.value;
        if (timeVal.includes('1 minuto')) timeout = 60;
        else if (timeVal.includes('5 minutos')) timeout = 300;
        else timeout = 0; // Unlimited

        try {
            const response = await fetch('/api/solve', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ period: period, timeout: timeout })
            });

            const data = await response.json();

            if (data.status === 'success') {
                statusMsg.textContent = "Horario generado con éxito!";
                statusMsg.style.color = "#4ade80"; // Green
                renderTimetable(data.solution);
                resultsSection.classList.remove('results-hidden');
            } else {
                statusMsg.textContent = "Error: " + data.message;
                statusMsg.style.color = "#f87171"; // Red
            }
        } catch (error) {
            statusMsg.textContent = "Error de conexión: " + error;
            statusMsg.style.color = "#f87171";
        }
    });

    function renderTimetable(solution) {
        // Group by Group ID
        const groups = {};
        solution.forEach(item => {
            if (!groups[item.group_id]) groups[item.group_id] = [];
            groups[item.group_id].push(item);
        });

        let html = '';
        for (const [groupId, items] of Object.entries(groups)) {
            html += `<div class="card"><h3>Grupo ${groupId}</h3>`;
            html += `<table><thead><tr><th>Materia</th><th>Profesor</th><th>Horario ID</th></tr></thead><tbody>`;
            items.forEach(item => {
                html += `<tr>
                    <td>${item.course_id}</td>
                    <td>${item.professor_id}</td>
                    <td>${item.timeslot_id}</td>
                </tr>`;
            });
            html += `</tbody></table></div>`;
        }
        timetableDisplay.innerHTML = html;
    }
});
