package com.gaejexperiments.db;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.jdo.PersistenceManager;
import javax.jdo.Query;

import com.google.appengine.api.search.SortExpression.SortDirection;

public class DBUtils {
	public static final Logger _logger = Logger.getLogger(DBUtils.class
			.getName());
	public static String getSensorValueMasterList() throws Exception {
		return "a,b,c";
	}
	public static void saveSensorReport(SensorReport sensorReport)
			throws Exception {
		PersistenceManager pm = PMF.get().getPersistenceManager();
		try {
			pm.makePersistent(sensorReport);
			_logger.log(Level.INFO, "Sensor Report has been saved");
		} catch (Exception ex) {
			_logger.log(
					Level.SEVERE,
					"Could not save the Sensor Report. Reason : "
							+ ex.getMessage());
			throw ex;
		} finally {
			pm.close();
		}
	}
	public static String [] getSensorValueForTagCode(String tagCode) {
		PersistenceManager pm = null;
		String xmlData1 = "";
		String xmlData2 = "";
		String xmlData3 = "";
		try {
			String[] sensorValues = {};
			if (tagCode.equalsIgnoreCase("ALL")) {
			} else {
				sensorValues = new String[] { tagCode };
			}
			pm = PMF.get().getPersistenceManager();
			Query query = null;
			if (tagCode.equalsIgnoreCase("ALL")) {
			} else {
				query = pm.newQuery(SensorReport.class,
						" tagCode == paramTagCode");
				query.setOrdering("reportDateTime");
				query.declareParameters("String paramTagCode");
			}
			for (int i = 0; i < sensorValues.length; i++) {
				List<SensorReport> codes = null;
				if (tagCode.equalsIgnoreCase("ALL")) {
				} else {
					codes = (List<SensorReport>) query
							.executeWithArray(tagCode);
				}
				int i_comma = 0;
				for (Iterator iterator = codes.iterator(); iterator.hasNext();) {
					SensorReport _report = (SensorReport) iterator.next();
					if (i_comma == 0){
						xmlData1 += "['"+ _report.getReportDateTime().toString() + "'," + _report.getSensor1Value()+ "]";
						xmlData2 += "['"+ _report.getReportDateTime().toString() + "'," + _report.getSensor2Value()+ "]";
						xmlData3 += "['"+ _report.getReportDateTime().toString() + "'," + _report.getSensor3Value()+ "]";
						i_comma++;
					}
					else{
						xmlData1 += ", ['"+_report.getReportDateTime() + "'," + _report.getSensor1Value()+ "]";
						xmlData2 += ", ['"+_report.getReportDateTime() + "'," + _report.getSensor2Value()+ "]";
						xmlData3 += ", ['"+_report.getReportDateTime() + "'," + _report.getSensor3Value()+ "]";
						i_comma++;
					}	
				}
			}
			String [] xmlData={xmlData1,xmlData2,xmlData3};
			return xmlData;
		} catch (Exception ex) {
			return null;
		} finally {
			pm.close();
		}
	}
}