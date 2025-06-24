import React, { useState, useEffect } from 'react';
import Chart from "chart.js/auto";
import { Pie } from 'react-chartjs-2';

const MemoryPagesChart = () => {
    const API_URL = import.meta.env.VITE_API_URL
    const ENPOINT_MPAGES = import.meta.env.VITE_ENDPOINT_MPAGES
    const [mpagesData, setMpagesData] = useState([50, 50])

    const updateMpagesData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENPOINT_MPAGES, requestOptions);
        const data = await response.json();
        setMpagesData([data.active_pages_mem, data.inactive_pages_mem])
    }

    useEffect(() => {
        // Esta función se ejecutará cada 1000 milisegundos (1 segundo)
        const intervalId = setInterval(() => {
            updateMpagesData();
        }, 1000);

        // Limpieza del intervalo cuando el componente se desmonta o actualiza
        return () => clearInterval(intervalId);
    }, []);

    const data = {
        labels: ['Activas', 'Inactivas'],
        datasets: [
            {
                label: 'Páginas del sistema',
                data: mpagesData,
                backgroundColor: ['rgba(160, 255, 125, 0.2)', 'rgba(255, 108, 108, 0.2)'],
                hoverBackgroundColor: ['rgba(160, 255, 125, 1)', 'rgba(255, 108, 108, 1)'],
            },
        ],
    }

    return (
        <>
            <div className='container mx-auto py-5'>
                <h1 className="text-center text-5xl my-8">Páginas del sistema</h1>
                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-7">
                    <h2 className="text-center text-3xl mb-2">Páginas activas e inactivas</h2>
                    <div className="py-2">
                        <Pie data={data} />
                    </div>
                </div>

            </div>
        </>
    );
}

export default MemoryPagesChart;