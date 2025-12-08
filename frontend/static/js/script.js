document.addEventListener('DOMContentLoaded', () => {
    const generateBtn = document.getElementById('generate-btn');
    const statusMsg = document.getElementById('status-message');
    const resultsSection = document.getElementById('results-section');
    const timetableDisplay = document.getElementById('timetable-display');
    const periodSelect = document.getElementById('period-select');
    const periodsData = JSON.parse(document.getElementById('periods-data').textContent);
    const timeLimitSelect = document.querySelector('select:nth-of-type(1)'); // Assuming first select in Generate section
    const loadingModal = document.getElementById('loading-modal');

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
        // Show Modal
        loadingModal.style.display = 'flex';
        statusMsg.textContent = "Generando horario...";
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
        } finally {
            // Hide Modal
            loadingModal.style.display = 'none';
        }
    });

    function renderTimetable(solution) {
        // 1. Enrich solution with extra data (Course Name, Professor Name, etc.)
        // We need to fetch this or map it. For now, we use IDs but we can try to map if data is available.
        // Ideally, the backend should return full objects, but we can do it here if we have the lists.

        // Group by Semester (Cuatrimestre)
        // We infer semester from Group ID (e.g. 301 -> 3)
        const semesters = {};

        solution.forEach(item => {
            // Heuristic for semester: First digit(s) of group_id
            let sem = Math.floor(item.group_id / 100);
            if (item.group_id >= 1000) sem = Math.floor(item.group_id / 100); // e.g. 1001 -> 10

            if (!semesters[sem]) semesters[sem] = [];
            semesters[sem].push(item);
        });

        let html = '';

        // Sort semesters
        Object.keys(semesters).sort((a, b) => a - b).forEach(sem => {
            const items = semesters[sem];
            html += `<div class="card" style="margin-bottom: 32px;">
                <h3 style="color: var(--primary); border-bottom: 2px solid #f3e8ff; padding-bottom: 12px; margin-bottom: 24px;">
                    Cuatrimestre ${sem}
                </h3>`;

            // Create Grid for this semester
            // We need to map timeslot_id to (Day, Hour)
            // We'll create a matrix: rows=TimeSlots(unique hours), cols=Days(Mon-Fri)

            // 1. Get unique time ranges
            // We need to know the time for each timeslot_id. 
            // Since we don't have the full timeslot objects here easily (unless we fetch them),
            // we will rely on the backend sending enriched data OR we fetch timeslots.
            // FOR NOW: We will assume standard timeslots 1-9 (Mon), 101-109 (Tue), etc.
            // and map them to a simple grid.

            html += `<div style="overflow-x: auto;">
                <table class="timetable-grid">
                    <thead>
                        <tr>
                            <th>Horario</th>
                            <th>Lunes</th>
                            <th>Martes</th>
                            <th>Miércoles</th>
                            <th>Jueves</th>
                            <th>Viernes</th>
                        </tr>
                    </thead>
                    <tbody>`;

            // Define standard hours (based on timeslots.json structure)
            const hours = [
                "7:00-7:55", "7:55-8:50", "8:50-9:45", "9:45-10:40",
                "11:10-12:05", "12:05-13:00", "13:00-13:55", "14:00-14:55", "14:55-15:50"
            ];

            // Base IDs for each day (assuming standard structure from timeslots.json)
            // L: 1-9, M: 101-109, Mi: 201-209, J: 301-309, V: 401-409
            const dayOffsets = [0, 100, 200, 300, 400];

            hours.forEach((timeRange, index) => {
                html += `<tr>
                    <td style="font-weight: 600; color: var(--text-gray); font-size: 0.8rem;">${timeRange}</td>`;

                for (let day = 0; day < 5; day++) {
                    // Calculate expected timeslot ID
                    // The IDs in timeslots.json are 1-based index + offset
                    // e.g. 7:00 Mon is ID 1. 7:00 Tue is ID 101.
                    const targetId = (index + 1) + dayOffsets[day];

                    // Find items in this slot
                    const cellItems = items.filter(i => i.timeslot_id === targetId);

                    html += `<td style="vertical-align: top; height: 80px;">`;
                    if (cellItems.length > 0) {
                        cellItems.forEach(ci => {
                            html += `<div class="session-block" style="background: #f3e8ff; border-left: 3px solid var(--primary); padding: 4px 8px; margin-bottom: 4px; border-radius: 4px; font-size: 0.75rem;">
                                <div style="font-weight: 700; color: var(--primary-dark);">${ci.course_name}</div>
                                <div style="color: var(--text-gray);">${ci.professor_name}</div>
                                <div style="font-size: 0.7rem; color: #94a3b8;">G: ${ci.group_id}</div>
                            </div>`;
                        });
                    }
                    html += `</td>`;
                }
                html += `</tr>`;
            });

            html += `</tbody></table></div></div>`;
        });

        timetableDisplay.innerHTML = html;
    }
});
